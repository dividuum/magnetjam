#include <stdio.h>

#include "video.h"
#include "misc.h"

void video_init(int fullscreen) {
    glfwInit();
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
    int mode = fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW;
    if(!glfwOpenWindow(1024, 768, 8,8,8,8, 0,0, mode))
        die("cannot open window");
    glfwSetWindowTitle("Magnet");
}

GLuint image_load(const char *filename) {
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#ifdef GL_GENERATE_MIPMAP_SGIS
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
#endif
    if (glfwLoadTexture2D(filename, GLFW_ORIGIN_UL_BIT|GLFW_BUILD_MIPMAPS_BIT))
        return tex_id;
    die("cannot load");
    return 0; // never reached
}

void video_shutdown() {
    glfwCloseWindow();
    glfwTerminate();
}
