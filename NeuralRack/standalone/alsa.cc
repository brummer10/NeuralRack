/*
 * alsa.cc
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2026 brummer <brummer@web.de>
 */

#include <alsa/asoundlib.h>
#include <thread>
#include <atomic>
#include <vector>

/****************************************************************
        alsa.cc   native ALSA support for NeuralRack
        
        this file is meant to be included in main.
****************************************************************/

snd_pcm_t* pcm_in = nullptr;
snd_pcm_t* pcm_out = nullptr;
snd_pcm_uframes_t bufferFrames = 256;
unsigned int sampleRate = 48000;

std::thread audioThread;
std::atomic<bool> runAlsaProcess{false};
std::vector<float> in_interleaved;
std::vector<float> out_interleaved;
std::vector<float> inL, inR;


static int setup_pcm(snd_pcm_t **handle, const char *device,snd_pcm_stream_t stream,
                                    unsigned int channels, snd_pcm_format_t format) {

    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);

    if (snd_pcm_open(handle, device, stream, 0) < 0) {
        fprintf(stderr, "Cannot open device %s\n", device);
        return -1;
    }

    snd_pcm_hw_params_any(*handle, params);
    snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(*handle, params, format);
    snd_pcm_hw_params_set_channels(*handle, params, channels);
    snd_pcm_hw_params_set_rate_near(*handle, params, &sampleRate, 0);
    snd_pcm_hw_params_set_period_size_near(*handle, params, &bufferFrames, 0);

    if (snd_pcm_hw_params(*handle, params) < 0) {
        fprintf(stderr, "Cannot set hw params\n");
        return -1;
    }

    snd_pcm_prepare(*handle);
    return 0;
}

void audio_loop() {
    while (runAlsaProcess.load()) {
        snd_pcm_sframes_t n = snd_pcm_readi(pcm_in, in_interleaved.data(), bufferFrames);

        if (n == -EPIPE) {
            snd_pcm_prepare(pcm_in);
            continue;
        }

        if (n <= 0) continue;

        for (unsigned int i = 0; i < bufferFrames; ++i) {
            float l = in_interleaved[i*2];
            inL[i] = l;
            inR[i] = l;
        }

        r->process(bufferFrames, inL.data(), inR.data());

        for (unsigned int i = 0; i < bufferFrames; ++i) {
            out_interleaved[i*2]     = inL[i];
            out_interleaved[i*2 + 1] = inR[i];
        }

        snd_pcm_sframes_t written = snd_pcm_writei(pcm_out, out_interleaved.data(), bufferFrames);
        if (written == -EPIPE) snd_pcm_prepare(pcm_out);
    }
}

void startAlsa() {
    if (setup_pcm(&pcm_in, "default", SND_PCM_STREAM_CAPTURE,
                            2, SND_PCM_FORMAT_FLOAT_LE) < 0) {
        fprintf(stderr, "Capture open failed\n");
        r->quitGui();
        return;
    }

    if (setup_pcm(&pcm_out, "default", SND_PCM_STREAM_PLAYBACK,
                            2, SND_PCM_FORMAT_FLOAT_LE) < 0) {
        fprintf(stderr, "Playback open failed\n");
        r->quitGui();
        return;
    }

    in_interleaved.resize(bufferFrames * 2);
    out_interleaved.resize(bufferFrames * 2);
    inL.resize(bufferFrames);
    inR.resize(bufferFrames);

    int prio = 25;
    r->initEngine(sampleRate, prio, 1);
    r->enableEngine(1);
    r->readConfig();

    runAlsaProcess = true;
    audioThread = std::thread(audio_loop);
    sched_param sch;
    sch.sched_priority = prio;
    if (pthread_setschedparam(audioThread.native_handle(), 1, &sch))
        fprintf(stderr, "ALSA Thread: fail to set priority\n");
    fprintf(stderr, "Running ALSA with %d Hz and %ld frames\n",
            sampleRate, bufferFrames);
}

void quitAlsa() {
    runAlsaProcess = false;

    if (audioThread.joinable())
        audioThread.join();

    if (pcm_in) {
        snd_pcm_close(pcm_in);
        pcm_in = nullptr;
    }

    if (pcm_out) {
        snd_pcm_close(pcm_out);
        pcm_out = nullptr;
    }
}
