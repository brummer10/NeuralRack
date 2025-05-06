
/*
 * xpa.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */

/****************************************************************
  XPa - a C++ wrapper for the portaudio server

  silent the portaudio device probe messages
  connection preference is set to 1.) jackd, 2.) alsa, 3.) pulse audio

****************************************************************/

#include <portaudio.h>

#if defined(__linux__) || defined(__FreeBSD__) || \
    defined(__NetBSD__) || defined(__OpenBSD__)
#include <pa_jack.h>
#include <pa_linux_alsa.h>
#endif

#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <stdio.h>

#pragma once

#ifndef XPA_H
#define XPA_H

class XPa {
public:

    XPa(const char* cname){
        #if defined(__linux__) || defined(__FreeBSD__) || \
            defined(__NetBSD__) || defined(__OpenBSD__)
        PaJack_SetClientName (cname);
        #endif
        init();
        SampleRate = 0;
    };

    ~XPa(){
        Pa_Terminate();
    };

    // open a audio stream for input/output channels and set the audio process callback
    bool openStream(uint32_t ichannels, uint32_t ochannels, PaStreamCallback *process, void* arg) {
        #if defined(__linux__) || defined(__FreeBSD__) || \
            defined(__NetBSD__) || defined(__OpenBSD__)
        std::vector<Devices> devices;
        int d = Pa_GetDeviceCount();
        const PaDeviceInfo *info;
        for (int i = 0; i<d;i++) {
            info = Pa_GetDeviceInfo(i);
            if ((std::strcmp(info->name, "pulse") ==0) ||
               (std::strcmp(info->name, "default") ==0) ||
               (std::strcmp(info->name, "system") ==0)) {
                Devices dev;
                if (std::strcmp(info->name, "pulse") ==0) dev.order = 3; // pulse audio
                if (std::strcmp(info->name, "default") ==0) dev.order = 2; // alsa
                if (std::strcmp(info->name, "system") ==0) dev.order = 1; // jackd
                dev.index = i;
                dev.SampleRate = SampleRate = info->defaultSampleRate;
                dev.Name = info->name;
                snprintf(dev.hostName, 63,"%s", getHostName(info->hostApi));
                devices.push_back(dev);
            }
            
        }

        std::sort(devices.begin(), devices.end(), 
        [](Devices const &a, Devices const &b) {
            return a.order < b.order; 
        });
        auto it = devices.begin();
        PaStreamParameters inputParameters;
        inputParameters.device = it->index;
        inputParameters.channelCount = ichannels;
        inputParameters.sampleFormat = paFloat32|paNonInterleaved;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        PaStreamParameters outputParameters;
        outputParameters.device = it->index;
        outputParameters.channelCount = ochannels;
        outputParameters.sampleFormat = paFloat32|paNonInterleaved;
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        bool isAlsa = std::strcmp(it->hostName, "ALSA") == 0 ;
        int frames = paFramesPerBufferUnspecified;
        if (isAlsa) {
            PaAlsa_EnableRealtimeScheduling(&stream, 1);
            it->SampleRate = SampleRate = 48000;
            frames = 256;
        } else if (std::strcmp(it->hostName, "JACK Audio Connection Kit") == 0) {
            #if defined(HAVE_JACK)
            devices.clear();
            Pa_Terminate();
            return false;
            #endif
        }

        err = Pa_OpenStream(&stream, ichannels ? &inputParameters : nullptr, 
                            ochannels ? &outputParameters : nullptr, it->SampleRate,
                            frames, paClipOff, process, arg);

        if (isAlsa) {
            std::cout << "using (" << it->Name << ") " << it->hostName 
            << " with " << frames << " frames per buffer and " << it->SampleRate 
            << "hz Sample Rate" << std::endl;
        }
        //std::cout << "using (" << it->Name << ") " << it->hostName 
        //    << " with " << it->SampleRate << "hz Sample Rate" << std::endl;

        devices.clear();
        #else
        const PaDeviceInfo *info;
        SampleRate = 48000;
        info = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());
        std::cout << "using (" << info->name << ") " << getHostName(info->hostApi) 
            << " with " << SampleRate << "hz Sample Rate" << std::endl;

        PaStreamParameters inputParameters;
        inputParameters.device = Pa_GetDefaultInputDevice();
        inputParameters.channelCount = ichannels;
        inputParameters.sampleFormat = paFloat32|paNonInterleaved;
        inputParameters.suggestedLatency = 0.050;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        PaStreamParameters outputParameters;
        outputParameters.device = Pa_GetDefaultOutputDevice();
        if (outputParameters.device == paNoDevice) return false;
        outputParameters.channelCount = ochannels;
        outputParameters.sampleFormat = paFloat32|paNonInterleaved;
        outputParameters.suggestedLatency = 0.050;
        outputParameters.hostApiSpecificStreamInfo = nullptr;
        //SampleRate = info->defaultSampleRate;
        err = Pa_OpenStream(&stream, ichannels ? &inputParameters : nullptr, 
                            ochannels ? &outputParameters : nullptr, SampleRate,
                            256, paClipOff, process, arg);
        #endif

        return err == paNoError ? true : false;
    }

    // start the audio processing
    bool startStream() {
        err = Pa_StartStream(stream);
        return err == paNoError ? true : false;
    }

    // helper function to get a pointer to the PaStream object
    PaStream* getStream() {
        return stream;
    }

    // helper function to get the SampleRate used by the audio sever
    uint32_t getSampleRate() {
        return SampleRate;
    }

    // stop the audio processing
    void stopStream() {
        if (Pa_IsStreamActive(stream)) {
            err = Pa_StopStream(stream);
            if (err != paNoError) {
                std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
            }
            err = Pa_CloseStream(stream);
            if (err != paNoError) {
                std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
            }
        }
    }

private:
    PaStream* stream;
    PaError err;
    uint32_t SampleRate;

    struct Devices {
        int order;
        int index;
        std::string Name;
        char hostName[64];
        uint32_t SampleRate;
    };

    const char* getHostName(unsigned int index){
        const PaHostApiInfo* info;
        uint32_t apicount =  Pa_GetHostApiCount();
        if(apicount <= 0) return "";
        if(index > apicount-1) return "";
        info =  Pa_GetHostApiInfo(index);
        return info->name;
    }

    // initialise the portaudio server,
    // catch the falling device probe messages  
    void init() {
        #if defined(__linux__) || defined(__FreeBSD__) || \
            defined(__NetBSD__) || defined(__OpenBSD__)
        char buffer[1024];
        auto fp = fmemopen(buffer, 1024, "w");
        if ( !fp ) { std::printf("error"); }
        auto old = stderr;
        stderr = fp;
        #endif
        Pa_Initialize();
        #if defined(__linux__) || defined(__FreeBSD__) || \
            defined(__NetBSD__) || defined(__OpenBSD__)
        std::fclose(fp);
        stderr = old;
        #endif
    }

};


#endif
