#ifndef AUDIO_H
#define AUDIO_H

#include <AL/al.h>

void audio_init(int argc, char *argv[]);
ALuint audio_load(const char *filename);
void audio_shutdown();

#endif 
