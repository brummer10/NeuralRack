/*
 * NeuralModelLoader.cc
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */


#include "NeuralModelLoader.h"


namespace neuralrack {

NeuralModelLoader::NeuralModelLoader(std::condition_variable *Sync)
    : model(nullptr), smp(), SyncWait(Sync) {
    NeuralAudio::NeuralModel::SetDefaultMaxAudioBufferSize(4096);
    loudness = 0.0;
    nGain = 1.0;
    needResample = 0;
    phaseOffset = 0;
    isInited = false;
    ready.store(false, std::memory_order_release);
    do_ramp.store(false, std::memory_order_release);
    do_ramp_down.store(false, std::memory_order_release);
}

NeuralModelLoader::~NeuralModelLoader() {
    if (model != nullptr) delete model;
}

void NeuralModelLoader::clearState() {

}

void NeuralModelLoader::init(unsigned int sample_rate) {
    fSampleRate = sample_rate;
    clearState();
    isInited = true;
    ramp = 0.0;
    ramp_step = 256.0;
    ramp_down = ramp_step;
    ramp_div = 1.0/ramp_step;
    loadModel();
}

// connect the Ports used by the plug-in class
void NeuralModelLoader::connect(uint32_t port,void* data) {

}

std::string NeuralModelLoader::getModelFile() {
    return modelFile;
}

void NeuralModelLoader::setModelFile(std::string modelFile_) {    
    modelFile = modelFile_;
}

void NeuralModelLoader::normalize(int count, float *buf) {
    if (!model) return;
    if (nGain != 1.0) {
        for (int i0 = 0; i0 < count; i0 = i0 + 1) {
            buf[i0] = float(double(buf[i0]) * nGain);
        }
    }
}

int NeuralModelLoader::getPhaseOffset() {
    return phaseOffset;
}

void NeuralModelLoader::compute(int count, float *input0, float *output0) {
    if (output0 != input0)
        memcpy(output0, input0, count*sizeof(float));

    // process model
    if (model && ready.load(std::memory_order_acquire)) {

        float buf[count];
        memcpy(buf, output0, count*sizeof(float));

        if (needResample ) {
            int ReCounta = count;
            if (needResample == 1) {
                ReCounta = smp.max_out_count(count);
            } else if (needResample == 2) {
                ReCounta = static_cast<int>(ceil((count*static_cast<double>(modelSampleRate))/fSampleRate));
            }
            float buf1[ReCounta];
            memset(buf1, 0, ReCounta*sizeof(float));
            if (needResample == 1) {
                ReCounta = smp.up(count, buf, buf1);
            } else if (needResample == 2) {
                smp.down(buf, buf1);
            } else {
                memcpy(buf1, buf, ReCounta * sizeof(float));
            }
            model->Process(buf1, buf1, ReCounta);

            if (needResample == 1) {
                smp.down(buf1, buf);
            } else if (needResample == 2) {
                smp.up(ReCounta, buf1, buf);
            }
        } else {
            model->Process(buf, buf, count);
        }
        memcpy(output0, buf, count*sizeof(float));

        if (do_ramp.load(std::memory_order_acquire)) {
            for (int i = 0; i < count; i++) {
                if (ramp < ramp_step) {
                    ++ramp;
                    output0[i] *= (ramp * ramp_div);
                } else {
                    do_ramp.store(false, std::memory_order_release);
                    ramp = 0.0;
                }
            }
        }
    }
    if (do_ramp_down.load(std::memory_order_acquire)) {
        for (int i = 0; i < count; i++) {
            if (ramp_down > 0.0) {
                --ramp_down;
            } else {
                SyncIntern.notify_all();
            }
            output0[i] *= (ramp_down * ramp_div);
        }
    }
}

// non rt callback
bool NeuralModelLoader::loadModel() {
    if (!modelFile.empty() && isInited) {
        if (model) {
            do_ramp_down.store(true, std::memory_order_release);
            std::unique_lock<std::mutex> lkr(WMutex);
            SyncIntern.wait_for(lkr, std::chrono::milliseconds(60));
        }
        //fprintf(stderr, "Load file %s\n", modelFile.c_str());
        ready.store(false, std::memory_order_release);
        std::unique_lock<std::mutex> lk(WMutex);
        SyncWait->wait_for(lk, std::chrono::milliseconds(60));
        if (model != nullptr) {
            delete model;
            model = nullptr;
        }
       // fprintf(stderr, "delete model\n");
        needResample = 0;
        phaseOffset = 0;
        //clearState();
        int32_t warmUpSize = 4096;
        try {
            model = NeuralAudio::NeuralModel::CreateFromFile(std::string(modelFile));
        } catch (const std::exception&) {
            modelFile = "None";
        }
        
        if (model) {
            //fprintf(stderr, "load model %s\n", modelFile.c_str());
            if (model->GetRecommendedOutputDBAdjustment()) {
                loudness = model->GetRecommendedOutputDBAdjustment();
                nGain = pow(10.0, (-6.0 - loudness) / 20.0);
            } else {
                nGain = 1.0;
            }
            modelSampleRate = static_cast<int>(model->GetSampleRate());
            if (modelSampleRate <= 0) modelSampleRate = 48000;
            if (modelSampleRate > fSampleRate) {
                smp.setup(fSampleRate, modelSampleRate);
                needResample = 1;
            } else if (modelSampleRate < fSampleRate) {
                smp.setup(modelSampleRate, fSampleRate);
                needResample = 2;
            }
            float* buffer = new float[warmUpSize];
            memset(buffer, 0, warmUpSize * sizeof(float));
            float angle = 0.0;
            for(int i=0;i<2048;i++){
                buffer[i] = sin(angle);
                angle += (2 * 3.14159365) / 2048;
            }

            model->Process(buffer, buffer, warmUpSize);

            for(int i=0;i<2048;i++){
                if (!std::signbit(buffer[i+1]) != !std::signbit(buffer[i])) {
                    phaseOffset = i;
                    break;
                }
            }

            delete[] buffer;
            model->Prewarm();
            //fprintf(stderr, "sample rate = %i file = %i l = %f\n",fSampleRate, modelSampleRate, loudness);
            //fprintf(stderr, "%s\n", load_file.c_str());
        }
        ramp = 0.0;
        ready.store(true, std::memory_order_release);
        do_ramp.store(true, std::memory_order_release);
        do_ramp_down.store(false, std::memory_order_release);
        ramp_down = ramp_step;
    }
    if (model) return true;
    return false;
}

// non rt callback
void NeuralModelLoader::unloadModel() {
    std::unique_lock<std::mutex> lk(WMutex);
    ready.store(false, std::memory_order_release);
    SyncWait->wait_for(lk, std::chrono::milliseconds(160));
    if (model != nullptr) {
        delete model;
        model = nullptr;
    }
   // fprintf(stderr, "delete model\n");
    needResample = 0;
    //clearState();
    modelFile = "None";
    ready.store(true, std::memory_order_release);
}

// clean up
void NeuralModelLoader::cleanUp() {
    ready.store(false, std::memory_order_release);
    if (model != nullptr) {
        delete model;
        model = nullptr;
    }
    needResample = 0;
    modelFile = "None";
    ready.store(true, std::memory_order_release);
}

} // end namespace neuralrack

