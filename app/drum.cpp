#include <iostream>
#include <cmath>
#include <alsa/asoundlib.h>
#include "daisysp.h"

using namespace daisysp;

const int sample_rate = 48000;
const int channels = 2;
const int buffer_frames = 512;

// ================== 官方 Drum ==================

AnalogBassDrum kick;
AnalogSnareDrum snare;
WhiteNoise noise;  // 用于简单 hi-hat
AdEnv hihat_env;

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

    kick.Init(sample_rate);
    kick.SetFreq(60);       // 低频
    kick.SetDecay(0.7f);

    snare.Init(sample_rate);
    snare.SetFreq(200);
    snare.SetDecay(0.5f);

    noise.Init();

    hihat_env.Init(sample_rate);
    hihat_env.SetTime(ADENV_SEG_ATTACK, 0.001);
    hihat_env.SetTime(ADENV_SEG_DECAY, 0.05);
    hihat_env.SetMin(0);
    hihat_env.SetMax(1);

    int step = 0;
    int counter = 0;

    std::cout << "Official DaisySP Drum Machine...\n";

    while (true)
    {
        for (int i = 0; i < buffer_frames; i++)
        {
            float sig = 0;

            // ===== 节奏 =====
            if (counter % (sample_rate / 2) == 0)
            {
                step++;

                if (step % 4 == 0)
                    kick.Trig();

                if (step % 4 == 2)
                    snare.Trig();

                hihat_env.Trigger();
            }

            // ===== 生成声音 =====
            float k = kick.Process();
            float s = snare.Process();
            float h = noise.Process() * hihat_env.Process();

            sig = k + s * 0.7f + h * 0.3f;

            sig = tanh(sig);

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