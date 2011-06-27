#define _GNU_SOURCE
#include <GL/glfw.h>
#include <AL/al.h>

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "misc.h"
#include "video.h"
#include "audio.h"
#include "font.h"

#define NUM_MAGNETS 10
#define NUM_BLOBS 500
#define NUM_GOALS 3
#define BLOBS_TO_WIN 100
#define SECONDS 180

static int win_w, win_h;
static int mouse_x, mouse_y;
static double end;
static int remaining_blobs;
static double score;
static enum { FAIL = -1, RUNNING = 0, WIN = 1 } status;

static ALuint 
    snd_goal,
    snd_fail,
    snd_win,
    snd_music
;

static GLuint
    gfx_blob,
    gfx_gate1,
    gfx_gate2,
    gfx_gate3,
    gfx_magnet
;



typedef struct {
    double x;
    double y;  
    double dir;
    double length;
    int active;
} magnet_t;

magnet_t magnets[NUM_MAGNETS];

typedef struct {
    double x;
    double y;
    double momentum_x;
    double momentum_y;
    int next_goal;
} blob_t;

blob_t blobs[NUM_BLOBS];

typedef struct {
    double x;
    double y;
    double radius;
} goal_t;

goal_t goals[NUM_GOALS];

int current_magnet = 0;

static void goal_color(int goal) {
    if (goal == 0) {
        glColor4f(1, 1, 0, 1);
    } else if (goal == 1) {
        glColor4f(0, 1, 1, 1);
    } else if (goal == 2) {
        glColor4f(1, 0, 1, 1);
    } else if (goal == 3) {
        glColor4f(1, 1, 1, 1);
    }
}

static void GLFWCALL reshape(int width, int height) {
    win_w = width;
    win_h = height;
}

static void GLFWCALL mousemotion(int x, int y) {
    mouse_x = x;
    mouse_y = y;

    magnet_t *magnet = magnets + current_magnet;
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)) {
        magnet->x = mouse_x;
        magnet->y = mouse_y;
    }
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT)) {
        double dx = mouse_x - magnet->x;
        double dy = mouse_y - magnet->y;
        magnet->dir = atan2(dx, dy);
        magnet->length = min(60, sqrt(dx*dx+dy*dy));
    }
}

static void GLFWCALL mousebutton(int button, int action) { 
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            double min_dist = 100000;
            int min_m = -1;
            for (int m = 0; m < NUM_MAGNETS; m++) {
                magnet_t *magnet = magnets+m;
                if (!magnet->active)
                    continue;
                double dx = mouse_x - magnet->x;
                double dy = mouse_y - magnet->y;
                double dist = sqrt(dx*dx+dy*dy);
                if (dist < min_dist) {
                    min_m = m;
                    min_dist = dist;
                }
            }
            if (min_dist < 50 && min_m != -1) {
                current_magnet = min_m;
            }
        }
    }
}

static void GLFWCALL keypressed(int key, int action) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            for (int m = 0; m < NUM_MAGNETS; m++) {
                magnet_t *magnet = magnets+m;
                if (magnet->active) 
                    continue;
                magnet->dir = 0;
                magnet->length = 30;
                magnet->x = mouse_x;
                magnet->y = mouse_y;
                magnet->active = 1;
                current_magnet = m;
                break;
            }
        } else if (key == GLFW_KEY_ESC) {
            running = 0;
        }
    }
}

void game_init() {
    score = 0;
    end = now + SECONDS * 1000;
    remaining_blobs = BLOBS_TO_WIN;
    status = RUNNING;

    snd_goal = audio_load("goal.wav");
    snd_fail = audio_load("fail.wav");
    snd_win = audio_load("win.wav");
    snd_music = audio_load("music.wav");

    gfx_blob = image_load("blob.tga");
    gfx_gate1 = image_load("gate1.tga");
    gfx_gate2 = image_load("gate2.tga");
    gfx_gate3 = image_load("gate3.tga");
    gfx_magnet = image_load("magnet.tga");

    alSourcei(snd_music, AL_LOOPING, AL_TRUE);
    alSourcef(snd_music, AL_GAIN, 0.7);
    alSourcePlay(snd_music);

    srandom(time(NULL));

    glfwEnable(GLFW_KEY_REPEAT);
    glfwDisable(GLFW_AUTO_POLL_EVENTS);
    glfwEnable(GLFW_STICKY_KEYS);

    glfwSwapInterval(1);
    glfwSetWindowSizeCallback(reshape);
    glfwSetMousePosCallback(mousemotion);
    glfwSetMouseButtonCallback(mousebutton);
    glfwSetKeyCallback(keypressed);

    for (int m = 0; m < NUM_MAGNETS; m++) {
        magnet_t *magnet = magnets+m;
        magnet->dir = 0;
        magnet->length = 50;
        magnet->x = win_w/2;
        magnet->y = win_h/2;
        magnet->active = m == 0;
    }

    for (int b = 0; b < NUM_BLOBS; b++) {
        blob_t *blob = blobs+b;
        //blob->x = b % 10 * 100;
        //blob->y = b / 10 * 100;
        blob->x = -1000000;
        blob->y = -1000000;
        blob->momentum_x = 0.0;
        blob->momentum_y = 0.0;
        blob->next_goal = 0;
    }

    for (int g = 0; g < NUM_GOALS; g++) {
        goal_t *goal = goals+g;
        goal->x = random() % (win_w - 400) + 200;
        goal->y = random() % (win_h - 400) + 200;
        goal->radius = 30;
    }
}

void calc_magnet(magnet_t *magnet, double point_x, double point_y, double *f_x, double *f_y) {
    double north_x = magnet->x + sin(magnet->dir)*magnet->length;
    double north_y = magnet->y + cos(magnet->dir)*magnet->length;

    double south_x = magnet->x - sin(magnet->dir)*magnet->length;
    double south_y = magnet->y - cos(magnet->dir)*magnet->length;



    double n_dx = point_x - north_x;
    double n_dy = point_y - north_y;

    double n_force_dir = atan2(n_dx, n_dy);
    double n_dist = sqrt(n_dx*n_dx + n_dy*n_dy);
    if (n_dist < 5) n_dist = 5;
    double n_force_strength = exp(-pow(n_dist/(magnet->length * 2), 2));

    double n_f_x = sin(n_force_dir) * n_force_strength;
    double n_f_y = cos(n_force_dir) * n_force_strength;



    double s_dx = point_x - south_x;
    double s_dy = point_y - south_y;

    double s_force_dir = atan2(s_dx, s_dy) + 3.141;
    double s_dist = sqrt(s_dx*s_dx + s_dy*s_dy);
    if (s_dist < 5) s_dist = 5;
    double s_force_strength = exp(-pow(s_dist/(magnet->length * 2), 2));

    double s_f_x = sin(s_force_dir) * s_force_strength;
    double s_f_y = cos(s_force_dir) * s_force_strength;


    *f_x += n_f_x + s_f_x;
    *f_y += n_f_y + s_f_y;
}

int remaining_to_spawn = 0;
int next_spawn = 0;

void endgame_tick(int delta) {
    if (status == WIN) {
        font_write(win_w/2 - 420, win_h / 2, 100, "Du Gewinnertyp!");
        font_write(win_w/2 - 130, win_h / 2 + 100, 50, "Score %.1f", score);
    } else {
        font_write(win_w/2 - 220, win_h / 2 + 100, 200, "Fail");
    }
}

void simulation_tick(int delta) {
    alListener3f(AL_POSITION, 0, 0, 0);
    remaining_to_spawn -= delta;
    while (remaining_to_spawn < 0) {
        remaining_to_spawn += 100;
        next_spawn++;
        if (next_spawn >= NUM_BLOBS)
            next_spawn = 0;
        blob_t *blob = blobs+next_spawn;
        blob->x = 100;
        blob->y = 100;
        blob->momentum_x = 0.0;
        blob->momentum_y = 0.0;
        blob->next_goal = 0;
    }


    // Magnetwimpel!
    glPointSize(1);
    glLineWidth(1);
    for (double x = 0; x < win_w; x+=20) {
        for (double y = 0; y < win_h; y+=20) {
            double f_x = 0, f_y = 0;

            for (int m = 0; m < NUM_MAGNETS; m++) {
                magnet_t *magnet = magnets+m;
                if (!magnet->active) 
                    continue;
                calc_magnet(magnet, x, y, &f_x, &f_y);
            }
            glColor4f(1, 0, 0, 0.3);
            glBegin(GL_POINTS);
                glVertex2f(x, y);
            glEnd();

            glColor4f(1, 1, 1, 0.1);
            glBegin(GL_LINES);
                glVertex2f(x, y);
                glVertex2f(x + f_x*50, y + f_y*50);
            glEnd();
        }
    }


    // Blobs
    glBindTexture(GL_TEXTURE_2D, gfx_blob);
    glEnable(GL_TEXTURE_2D);
    for (int b = 0; b < NUM_BLOBS; b++) {
        blob_t *blob = blobs+b;

        blob->momentum_x *= 1.0;
        blob->momentum_y *= 1.0;

        if (blob->next_goal < NUM_GOALS) {
            double f_x = 0, f_y = 0;
            for (int m = 0; m < NUM_MAGNETS; m++) {
                magnet_t *magnet = magnets+m;
                if (!magnet->active) 
                    continue;
                calc_magnet(magnet, blob->x, blob->y, &f_x, &f_y);
            }
            blob->momentum_x += f_x;
            blob->momentum_y += f_y;
        }
        blob->x += blob->momentum_x;
        blob->y += blob->momentum_y;

        for (int g = 0; g < NUM_GOALS; g++) {
            goal_t *goal = goals + g;
            double dx = goal->x - blob->x;
            double dy = goal->y - blob->y;
            double dist = sqrt(dx*dx + dy*dy);
            if (dist < goal->radius && blob->next_goal == g) {
                alSourcef(snd_goal, AL_PITCH, g+1.0); 
                alSourcef(snd_goal, AL_GAIN, g+1.0);
                alSourcePlay(snd_goal);

                blob->next_goal++;
                if (blob->next_goal >= NUM_GOALS) {
                    blob->momentum_x = sin(now*0.05) * 3;
                    blob->momentum_y = cos(now*0.05) * 3;

                    end += 1;
                    remaining_blobs--;
                    if (remaining_blobs == 0) {
                        alSourcePlay(snd_win);
                        status = WIN;
                    }
                }
            }
        }

        goal_color(blob->next_goal);
        glPushMatrix();
            glTranslatef(blob->x, blob->y, 0);
            glScalef(8, 8, 1);
            glRotatef(now, 0, 0, 1);
            glBegin(GL_QUADS);
                glTexCoord2f(0, 1);
                glVertex2i(-1, 1);

                glTexCoord2f(1, 1);
                glVertex2i(1, 1);

                glTexCoord2f(1, 0);
                glVertex2i(1, -1);

                glTexCoord2f(0, 0);
                glVertex2i(-1, -1);
            glEnd();
        glPopMatrix();
    }


    // Goals
    for (int g = 0; g < NUM_GOALS; g++) {
        goal_t *goal = goals+g;
        goal_color(g);
        GLuint textures[] = {gfx_gate1, gfx_gate2, gfx_gate3};
        glBindTexture(GL_TEXTURE_2D, textures[g]);
        glPushMatrix();
            glTranslatef(goal->x, goal->y, 0);
            glScalef(20, 20, 1);
            glRotatef(now * 0.2, 0, 0, 1);
            glBegin(GL_QUADS);
                glTexCoord2f(0, 1);
                glVertex2i(-1, 1);

                glTexCoord2f(1, 1);
                glVertex2i(1, 1);

                glTexCoord2f(1, 0);
                glVertex2i(1, -1);

                glTexCoord2f(0, 0);
                glVertex2i(-1, -1);
            glEnd();
        glPopMatrix();
    }


    // Magnete
    glBindTexture(GL_TEXTURE_2D, gfx_magnet);
    for (int m = 0; m < NUM_MAGNETS; m++) {
        magnet_t *magnet = magnets+m;
        glLineWidth(magnet->length);
        if (!magnet->active)
            continue;
        double alpha = m == current_magnet ? 0.9 : 0.5;

        glColor4f(1, 1, 1, alpha);
        glPushMatrix();
            glTranslatef(magnet->x, magnet->y, 0);
            glScalef(magnet->length, magnet->length, 1);
            glRotatef(90 - magnet->dir / M_PI * 180, 0, 0, 1);
            glBegin(GL_QUADS);
                glTexCoord2f(0, 1);
                glVertex2i(-1, 1);

                glTexCoord2f(1, 1);
                glVertex2i(1, 1);

                glTexCoord2f(1, 0);
                glVertex2i(1, -1);

                glTexCoord2f(0, 0);
                glVertex2i(-1, -1);
            glEnd();
        glPopMatrix();
    }
    glDisable(GL_TEXTURE_2D);
    
    double remaining = end - now;
    if (remaining < 0) {
        status = FAIL;
        alSourcePlay(snd_fail);
    }
    int sec = (int)(remaining / 1000) % 60;
    int min = (int)(remaining / 1000) / 60;

    score = remaining / 1000 + (BLOBS_TO_WIN - remaining_blobs);
    font_write(5, 20, 20, "%d:%02d bleiben dir um %d durchzuschleusen!", min, sec, remaining_blobs);
    font_write(win_w - 150, 20, 20, "Score: %.1f", score);
    font_write(5, win_h - 5, 20, "<click> verschieben, <right-click> drehen, <space> neuer magnet");
}

void game_tick(int delta) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, win_w, win_h);
    glOrtho(0, win_w,
            win_h, 0,
            -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glfwPollEvents();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

    glBindTexture(GL_TEXTURE_2D, 0);

    if (status == RUNNING) {
        simulation_tick(delta);
    } else {
        endgame_tick(delta);
    }

    glfwSwapBuffers();

    if (!glfwGetWindowParam(GLFW_OPENED))
        running = 0;
}

