#include <stdio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include "misc.h"

void audio_init(int argc, char *argv[]) {
    alutInit(&argc, argv);
}

ALuint audio_load(const char *filename) {
    ALuint source, buffer;
    buffer = alutCreateBufferFromFile(filename);
    if (alutGetError() != ALUT_ERROR_NO_ERROR)
        die("unable to load sound");

    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    return source;
}

void audio_shutdown() {
    alutExit();
}

