
/*
 * NeuralModelLoaderLoader.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */

#ifdef _WIN32
#define MINGW_STDTHREAD_REDUNDANCY_WARNING
#endif

#include <atomic>
#include <cstdint>
#include <unistd.h>
#include <mutex>
#include <cstring>
#include <condition_variable>

#include "NeuralModel.h"

#include "gx_resampler.h"

#pragma once

#ifndef NEURAL_MODEL_LOADER_H_
#define NEURAL_MODEL_LOADER_H_


namespace neuralrack {

class NeuralModelLoader  {
private:
    NeuralAudio::NeuralModel*       model;
    gx_resample::FixedRateResampler smp;

    std::atomic<bool>               ready;
    std::atomic<bool>               do_ramp;
    std::atomic<bool>               do_ramp_down;

    int                             fSampleRate;
    int                             modelSampleRate;
    int                             needResample;

    float                           loudness;
    float                           ramp;
    float                           ramp_down;
    float                           ramp_step;
    float                           ramp_div;

    bool                            isInited;
    std::mutex                      WMutex;
    std::condition_variable*        SyncWait;
    std::condition_variable         SyncIntern;

public:
    std::string                     modelFile;
    float                           nGain;
    int                             phaseOffset;

    void setModelFile(std::string modelFile_);
    std::string getModelFile() ;
    int getPhaseOffset() ;
    void clearState() ;
    void setMaxBufferSize(int maxSize) ;
    void init(unsigned int sample_rate) ;
    void connect(uint32_t port,void* data) ;
    void normalize(int count, float *buf) ;
    void compute(int count, float *input0, float *output0) ;
    bool loadModel() ;
    void unloadModel() ;
    void cleanUp() ;

    NeuralModelLoader(std::condition_variable *var);
    ~NeuralModelLoader();
};


} // end namespace neuralrack
#endif // NEURAL_MODEL_LOADER_H_
