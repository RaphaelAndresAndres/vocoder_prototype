#include <stdio.h>
#include <pthread.h>

// run with
// gcc -o main main.c GLMethods.c AudioMethods.c -lGL -lm -lglfw -lGLEW -lportaudio -lportmidi -lfftw3f -O3 -march=native
#include "GLMethods.h"
#include "AudioMethods.h"

int main(void)
{
    float *source = initAudio();
    pthread_t GL_id;
    pthread_create(&GL_id, NULL, initGL, (void *)source);
    pthread_join(GL_id, NULL);
    terminateAudio();
    return 0;
}