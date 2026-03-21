#include <iostream>
#include <cmath>
#include "portaudio.h"
#include "daisysp.h"

using namespace daisysp;

Oscillator osc;

static int audioCallback(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData)
{
    float *out = (float*)output;

    for (unsigned int i = 0; i < frameCount; i++)
    {
        float sig = osc.Process();

        out[i * 2]     = sig; // L
        out[i * 2 + 1] = sig; // R
    }

    return paContinue;
}

int main()
{
    Pa_Initialize();

    osc.Init(48000);
    osc.SetWaveform(Oscillator::WAVE_SIN);
    osc.SetFreq(440);

    PaStream *stream;

    Pa_OpenDefaultStream(
        &stream,
        0,
        2,
        paFloat32,
        48000,
        256,
        audioCallback,
        NULL);

    Pa_StartStream(stream);

    std::cout << "Playing... press Enter to stop\n";
    std::cin.get();

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}