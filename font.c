#include <GL/glfw.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "video.h"
#include "misc.h"
#include "font_stroke.h"

void font_write(GLdouble x, GLdouble y, GLdouble scale, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    const SFG_StrokeFont *font = &fgStrokeMonoRoman;
    scale /= font->Height;
    glLineWidth(1);
    glColor4f(1, 1, 1, 0.8);
    glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(scale, scale, 1);
        for (const char *c = buf; *c; c++) {
            if (*c >= font->Quantity) 
                continue;
            const SFG_StrokeChar* fchar = font->Characters[*c];
            const SFG_StrokeStrip *strip = fchar->Strips;
            for (int s = 0; s < fchar->Number; s++, strip++) {
                glBegin(GL_LINE_STRIP);
                for (int l = 0; l < strip->Number; l++) {
                    glVertex2f(strip->Vertices[l].X, -strip->Vertices[l].Y);
                }
                glEnd();
            }
            glTranslatef(fchar->Right*0.8, 0, 0);
        }
    glPopMatrix();
}
