#include <portaudio.h>
#include <portmidi.h>

#ifndef AUDIO_FUNCTIONS_H_INCLUDED
#define AUDIO_FUNCTIONS_H_INCLUDED
void *initMidi(void *);
float *initAudio();
void initAudioDevice();
void initFFT();
void runFFT(float *in, float *out);
void terminateAudio();
int WaveCallback(const void *inputBuffer,
                 void *outputBuffer,
                 unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo *timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void *userData);
void addKey(float freq, float volume);
float filterFreq(float freq);
#endif