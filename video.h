#ifndef VIDEO_H
#define VIDEO_H

#include <GL/glfw.h>

void video_init(int fullscreen);
GLuint image_load(const char *filename);
void video_shutdown();

#endif
