#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <GL/glfw.h>

#include "audio.h"
#include "video.h"
#include "game.h"

#define MIN_DELTA 30
#define MAX_DELTA 200

int running = 1;
double now = 0;

int main(int argc, char *argv[]) {
    video_init(0);
    audio_init(argc, argv);
    game_init();

    double last = glfwGetTime();

    while (running) {
        double current = glfwGetTime();
        int delta = (current - last) * 1000;
        if (delta < MIN_DELTA) {
            glfwSleep((MIN_DELTA-delta)/1000.0);
            continue;
        }

        last = current;
        now += delta;
        if (delta > MAX_DELTA)
            continue;

        game_tick(delta);
    }

    audio_shutdown();
    video_shutdown();
    return EXIT_SUCCESS;
}


