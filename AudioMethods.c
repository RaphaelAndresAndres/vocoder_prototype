#include <portaudio.h>
#include <math.h>
#include <fftw3.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "AudioMethods.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 2048
#define INDEX_FREQ_RATIO SAMPLE_RATE / FRAMES_PER_BUFFER
#define MAX_KEYS 10
#ifndef M_PI
#define M_PI 3.141592653
#endif

fftwf_plan PlanForward;
fftwf_plan PlanBackward;

fftwf_complex FFTOut[FRAMES_PER_BUFFER / 2 + 1];
static float staticInputBuffer[FRAMES_PER_BUFFER];
static float staticOutputBuffer[FRAMES_PER_BUFFER];
static float Amplitudes[FRAMES_PER_BUFFER / 2 + 1];
PaStream *stream;
float phase = 0;
float frequency = 200.;
float keyFrequencies[MAX_KEYS];

void shiftFreq(float freq)
{
    // adjust the FFT spectrum
    float curMax = 0.;
    int maxIndex = 0;
    float s = 0;
    fftwf_complex newFFTOut[FRAMES_PER_BUFFER / 2 + 1];
    for (int i = 1; i < FRAMES_PER_BUFFER / 2 + 1; ++i)
    {
        float freq = INDEX_FREQ_RATIO * (float)i;
        float curVolume = filterFreq(freq);
        FFTOut[i][0] *= curVolume;
        FFTOut[i][1] *= curVolume;

        Amplitudes[i] = (FFTOut[i][0] * FFTOut[i][0] + FFTOut[i][1] * FFTOut[i][1]);
        s += Amplitudes[i];
        if (curMax < Amplitudes[i])
        {
            curMax = Amplitudes[i];
            maxIndex = i;
        }
        newFFTOut[i][0] = 0;
        newFFTOut[i][1] = 0;
    }
    float maxFreq = (float)INDEX_FREQ_RATIO * (float)maxIndex;
    if (s == 0)
        printf("Skipped frame!\n");
    int limit = (int)(freq / maxFreq * (FRAMES_PER_BUFFER / 2 + 1));
    limit = FRAMES_PER_BUFFER / 2;
    for (int i = 1; i < limit; ++i)
    {
        float v = (float)i * maxFreq / freq;
        int lower = (int)floor(v);
        int upper = (int)ceil(v);
        if (lower > (FRAMES_PER_BUFFER / 2))
            break;
        if (lower == upper)
        {

            newFFTOut[i][0] = FFTOut[lower][0];
            newFFTOut[i][1] = FFTOut[lower][1];
            continue;
        }
        newFFTOut[i][0] = FFTOut[lower][0] + (FFTOut[upper][0] - FFTOut[lower][0]) * (v - (float)lower);
        newFFTOut[i][1] = FFTOut[lower][1] + (FFTOut[upper][1] - FFTOut[lower][1]) * (v - (float)lower);
        s += newFFTOut[i][0] + newFFTOut[i][1];
    }
    memcpy(FFTOut, newFFTOut, (FRAMES_PER_BUFFER / 2 + 1) * sizeof(fftwf_complex));
}

int WaveCallback(const void *inputBuffer,
                 void *outputBuffer,
                 unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo *timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void *userData)
{
    (void)framesPerBuffer;
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;
    float *in = (float *)inputBuffer;
    float *out = (float *)outputBuffer;

    runFFT(in, out);
    return paContinue;
}

float filterFreq(float freq)
{
    float lowerCutOff = 15.;
    float lowerMax = 50.;
    float upperMax = 3000;
    float upperCutOff = 4000;
    if (freq < lowerCutOff)
        return 0.;
    else if (freq < lowerMax)
        return (freq - lowerCutOff) / (lowerMax - lowerCutOff);
    else if (freq < upperMax)
        return 1.;
    else if (freq < upperCutOff)
        return -(freq - upperMax) / (upperCutOff - upperMax) + 1.;
    return 0.;
}

void runFFT(float *in, float *out)
{
    memcpy(staticInputBuffer, in, FRAMES_PER_BUFFER * sizeof(float));
    fftwf_execute(PlanForward);
    shiftFreq(frequency);
    fftwf_execute(PlanBackward);
    for (int i = 0; i < FRAMES_PER_BUFFER; ++i)
    {
        staticOutputBuffer[i] /= (float)FRAMES_PER_BUFFER;
    }
    memcpy(out, staticOutputBuffer, FRAMES_PER_BUFFER * sizeof(float));
}

void addKey(float freq, float volume)
{
    frequency = freq;
    printf("%f, %f\n", freq, volume);
}

void *initMidi(void *arg)
{
    (void)arg;
    printf("Setting up midi device...\n");
    Pm_Initialize();
    PmStream *midi_stream;
    int device_id = 3;
    const int midiBufferLength = 16;
    PmEvent buffer[midiBufferLength];
    Pm_OpenInput(&midi_stream, device_id, NULL, midiBufferLength, NULL, NULL);
    while (1)
    {
        int num_events =
            Pm_Read(midi_stream, buffer, midiBufferLength);
        if (num_events > 0)
        {
            for (int i = 0; i < num_events; i++)
            {
                float frequency = 27.5f * pow(2.f, (float)(((buffer[i].message >> 8) & 0xFF) - 21) / 12.);
                float volume = (float)((buffer[i].message >> 16) & 0xFF) / 128.;
                if ((buffer[i].message & 0xFF) == 144)
                    addKey(frequency, volume);
            }
        }
        Pa_Sleep(10);
    }
}

void initFFT()
{
    PlanForward = fftwf_plan_dft_r2c_1d(FRAMES_PER_BUFFER, staticInputBuffer, FFTOut, FFTW_MEASURE);
    PlanBackward = fftwf_plan_dft_c2r_1d(FRAMES_PER_BUFFER, FFTOut, staticOutputBuffer, FFTW_MEASURE);
}

void initAudioDevice()
{
    Pa_Initialize();
    Pa_OpenDefaultStream(&stream,
                         1,
                         1,
                         paFloat32,
                         SAMPLE_RATE,
                         FRAMES_PER_BUFFER,
                         WaveCallback,
                         NULL);
    Pa_StartStream(stream);
    printf("\e[1;1H\e[2J");
}

void terminateAudio()
{
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    printf("Audio has been terminated successfully.\n");
}

float *initAudio()
{
    initFFT();
    for (int i = 0; i < FRAMES_PER_BUFFER; ++i)
    {
        staticOutputBuffer[i] = 0;
    }

    printf("Setting up audio...\n");
    pthread_t MIDI_id;
    initAudioDevice();
    pthread_create(&MIDI_id, NULL, initMidi, NULL);
    return Amplitudes;
}
