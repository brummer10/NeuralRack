/*
 * ModelerSelector.h
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

#include "dsp.h"
#include "get_dsp.h"
#include "activations.h"

#include "RTNeural.h"

#include "gx_resampler.h"

#pragma once

#ifndef MODELER_SELECTOR_H_
#define MODELER_SELECTOR_H_


namespace neuralrack {


/****************************************************************
 ** ModelerBase - virtual base class to handle neural model loading and processing
 */

class ModelerBase {
public:
    virtual void setModelFile(std::string modelFile_) {}
    virtual inline std::string getModelFile() {return "";}
    virtual int getPhaseOffset() {return 0;}
    virtual inline void clearState() {}
    virtual inline void init(unsigned int sample_rate) {}
    virtual void connect(uint32_t port,void* data) {}
    virtual inline void normalize(int count, float *buf) {}
    virtual inline void compute(int count, float *input0, float *output0) {}
    virtual bool loadModel() { return false;}
    virtual void unloadModel() {}
    virtual void cleanUp() {}

    ModelerBase() {};
    virtual ~ModelerBase() {};
};


/****************************************************************
 ** NeuralModel - class to handle *.nam neural model loading and processing
 */

class NeuralModel : public ModelerBase {
private:
    nam::DSP*                       rawModel;
    std::unique_ptr<nam::DSP>       model;
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

    void setModelFile(std::string modelFile_) override { modelFile = modelFile_;}
    inline std::string getModelFile() override;
    int getPhaseOffset() override;
    inline void clearState() override;
    inline void init(unsigned int sample_rate) override;
    void connect(uint32_t port,void* data) override;
    inline void normalize(int count, float *buf) override;
    inline void compute(int count, float *input0, float *output0) override;
    bool loadModel() override;
    void unloadModel() override;
    void cleanUp() override;

    NeuralModel(std::condition_variable *var);
    ~NeuralModel();
};


/****************************************************************
 ** RtNeuralModel - class to handle *.json / *.aidax neural model loading and processing
 */

class RtNeuralModel : public ModelerBase {
private:
    RTNeural::Model<float>*         rawModel;
    std::unique_ptr<RTNeural::Model<float>>   model;
    gx_resample::FixedRateResampler smp;

    std::atomic<bool>               ready;
    std::atomic<bool>               do_ramp;
    std::atomic<bool>               do_ramp_down;

    int                             fSampleRate;
    int                             modelSampleRate;
    int                             needResample;

    float                           ramp;
    float                           ramp_down;
    float                           ramp_step;
    float                           ramp_div;

    bool                            isInited;
    std::mutex                      WMutex;
    std::condition_variable*        SyncWait;
    std::condition_variable         SyncIntern;

    void get_samplerate(std::string config_file, int *mSampleRate);

public:
    std::string                     modelFile;
    int                             phaseOffset;

    void setModelFile(std::string modelFile_) override { modelFile = modelFile_;}
    inline std::string getModelFile() override;
    int getPhaseOffset() override;
    inline void clearState() override;
    inline void init(unsigned int sample_rate) override;
    void connect(uint32_t port,void* data) override;
    inline void normalize(int count, float *buf) override;
    inline void compute(int count, float *input0, float *output0) override;
    bool loadModel() override;
    void unloadModel() override;
    void cleanUp() override;

    RtNeuralModel(std::condition_variable *var);
    ~RtNeuralModel();
};


/****************************************************************
 ** ModlerSelector - class to set neural modeler according to the file to load 
 */

class ModelerSelector {
public:

    void setModelFile(std::string modelFile_) {
            if (needNewModeler(modelFile_)) {
                selectModeler();
                modeler->init(sampleRate);
            }
            return modeler->setModelFile(modelFile_);}

    inline std::string getModelFile() {
        return modeler->getModelFile();
    }

    inline void clearState() {
            return modeler->clearState();}

    inline void init(unsigned int sample_rate) {
            sampleRate = sample_rate;
            return modeler->init(sample_rate);}

    void connect(uint32_t port,void* data) {
            return modeler->connect(port, data);}

    inline void normalize(int count, float *buf) {
            return modeler->normalize(count, buf); }

    inline void compute(int count, float *input0, float *output0) {
            return modeler->compute(count, input0, output0);}

    bool loadModel() {
            return modeler->loadModel();}

    int getPhaseOffset() {
            return modeler->getPhaseOffset();}

    void unloadModel() {
            return modeler->unloadModel();}

    void cleanUp() {
            return modeler->cleanUp();}

    ModelerSelector(std::condition_variable *var) :
            noModel(),
            namModel(var),
            rtnModel(var) {
            modeler = &noModel;
            sampleRate = 0;
            isNam = 3;}

    ~ModelerSelector() {}

private:
    ModelerBase *modeler;
    ModelerBase noModel;
    NeuralModel namModel;
    RtNeuralModel rtnModel;

    uint32_t sampleRate;
    int isNam;

    void selectModeler() {
            isNam ?
            modeler = dynamic_cast<ModelerBase*>(&namModel) :
            modeler = dynamic_cast<ModelerBase*>(&rtnModel);}

    bool needNewModeler(std::string newModelFile) {
            bool ret = true;
            int set = 0;
            std::string::size_type idx;
            std::string newExtension;
            idx = newModelFile.rfind('.');
            if(idx != std::string::npos)
                newExtension = newModelFile.substr(idx+1);
            if (!newExtension.empty()) {
                if (newExtension.compare("nam")) set = 0;
                else if (newExtension.compare("json")) set = 1;
                else if (newExtension.compare("aidax")) set = 1;
                ret = (isNam == set) ? false : true;
                isNam = set;
            }
            return ret;}

};

} // end namespace neuralrack
#endif // MODELER_SELECTOR_H_
