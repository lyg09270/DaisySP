#include <iostream>
#include <cmath>
#include <alsa/asoundlib.h>
#include "daisysp.h"

using namespace daisysp;

const int sample_rate = 48000;
const int channels = 2;
const int buffer_frames = 512;

// ================== Drum Synth ==================

struct Kick_t {
    float phase = 0;

    float Process()
    {
        phase += 1.0f / sample_rate;

        float env = expf(-phase * 8);
        float freq = 120.0f * (1 - phase);

        return sinf(2 * M_PI * freq * phase) * env;
    }

    void Trigger() { phase = 0; }
};

struct Snare_t {
    WhiteNoise noise;
    AdEnv env;

    void Init()
    {
        noise.Init();

        env.Init(sample_rate);
        env.SetTime(ADENV_SEG_ATTACK, 0.001);
        env.SetTime(ADENV_SEG_DECAY, 0.15);
        env.SetMin(0);
        env.SetMax(1);
    }

    float Process()
    {
        return noise.Process() * env.Process();
    }

    void Trigger() { env.Trigger(); }
};

struct HiHat_t {
    WhiteNoise noise;
    AdEnv env;

    void Init()
    {
        noise.Init();

        env.Init(sample_rate);
        env.SetTime(ADENV_SEG_ATTACK, 0.001);
        env.SetTime(ADENV_SEG_DECAY, 0.05);
        env.SetMin(0);
        env.SetMax(1);
    }

    float Process()
    {
        float n = noise.Process();
        return n * env.Process();
    }

    void Trigger() { env.Trigger(); }
};

// ================== ALSA ==================

int main()
{
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;

    snd_pcm_open(&handle, "plughw:1,0", SND_PCM_STREAM_PLAYBACK, 0);

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);

    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, channels);
    snd_pcm_hw_params_set_rate(handle, params, sample_rate, 0);

    snd_pcm_hw_params(handle, params);

    int16_t buffer[buffer_frames * channels];

    // ================== 初始化 ==================

    Kick_t kick;
    Snare_t snare;
    HiHat_t hihat;

    snare.Init();
    hihat.Init();

    int step = 0;
    int counter = 0;

    std::cout << "Drum machine running...\n";

    while (true)
    {
        for (int i = 0; i < buffer_frames; i++)
        {
            float sig = 0;

            // ===== 简单节奏 =====
            if (counter % (sample_rate / 2) == 0) // 每0.5秒
            {
                step++;

                if (step % 4 == 0) kick.Trigger();
                if (step % 4 == 2) snare.Trigger();
                hihat.Trigger();
            }

            sig += kick.Process();
            sig += snare.Process() * 0.5;
            sig += hihat.Process() * 0.3;

            sig = tanh(sig); // soft clip

            int16_t sample = sig * 32767;

            buffer[i * 2]     = sample;
            buffer[i * 2 + 1] = sample;

            counter++;
        }

        int err = snd_pcm_writei(handle, buffer, buffer_frames);

        if (err == -EPIPE)
        {
            snd_pcm_prepare(handle);
        }
    }

    snd_pcm_close(handle);
    return 0;
}