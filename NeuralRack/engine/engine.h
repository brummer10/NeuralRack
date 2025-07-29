/*
 * engine.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */


#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>

#ifdef __SSE__
 #include <immintrin.h>
 #ifndef _IMMINTRIN_H_INCLUDED
  #include <fxsrintrin.h>
 #endif
 #ifdef __SSE3__
  #ifndef _PMMINTRIN_H_INCLUDED
   #include <pmmintrin.h>
  #endif
 #else
  #ifndef _XMMINTRIN_H_INCLUDED
   #include <xmmintrin.h>
  #endif
 #endif //__SSE3__
#endif //__SSE__

#include "dcblocker.cc"
#include "eq.cc"
#include "NoiseGate.cc"

#include "NeuralModelLoader.h"
#include "fftconvolver.h"

#pragma once

#ifndef ENGINE_H_
#define ENGINE_H_
namespace neuralrack {

/////////////////////////// DENORMAL PROTECTION   //////////////////////

class DenormalProtection {
private:
#ifdef USE_SSE
    uint32_t  mxcsr_mask;
    uint32_t  mxcsr;
    uint32_t  old_mxcsr;
#endif

public:
    inline void set_() {
#ifdef USE_SSE
        old_mxcsr = _mm_getcsr();
        mxcsr = old_mxcsr;
        _mm_setcsr((mxcsr | _MM_DENORMALS_ZERO_MASK | _MM_FLUSH_ZERO_MASK) & mxcsr_mask);
#endif
    };
    inline void reset_() {
#ifdef USE_SSE
        _mm_setcsr(old_mxcsr);
#endif
    };

    inline DenormalProtection() {
#ifdef USE_SSE
        mxcsr_mask = 0xffbf; // Default MXCSR mask
        mxcsr      = 0;
        uint8_t fxsave[512] __attribute__ ((aligned (16))); // Structure for storing FPU state with FXSAVE command

        memset(fxsave, 0, sizeof(fxsave));
        __builtin_ia32_fxsave(&fxsave);
        uint32_t mask = *(reinterpret_cast<uint32_t *>(&fxsave[0x1c])); // Obtain the MXCSR mask from FXSAVE structure
        if (mask != 0)
            mxcsr_mask = mask;
#endif
    };

    inline ~DenormalProtection() {};
};

class Engine
{
public:
    ParallelThread               xrworker;
    NeuralModelLoader            slotA;
    NeuralModelLoader            slotB;
    ConvolverSelector            conv;
    ConvolverSelector            conv1;
    eq::Dsp*                     peq;
    noisegate::Dsp*              ngate;

    float                        inputGain;
    float                        inputGain1;
    float                        outputGain;
    float                        outputGain1;
    float                        IRoutputGain;
    float                        IRoutputGain1;
    float                        MasterOutGain;
    float                        IRmix;
    float                        buffered;
    float                        latency;
    float                        XrunCounter;

    int32_t                      normSlotA;
    int32_t                      normSlotB;
    int32_t                      rt_prio;
    int32_t                      rt_policy;
    uint32_t                     bypass;
    uint32_t                     s_rate;
    uint32_t                     bufsize;
    uint32_t                     buffersize;
    uint32_t                     eqOnOff;
    uint32_t                     ngOnOff;
    uint32_t                     IRmode;
    int                          phaseOffset;

    std::string                  model_file;
    std::string                  model_file1;

    std::string                  ir_file;
    std::string                  ir_file1;

    std::atomic<bool>            _execute;
    std::atomic<bool>            _notify_ui;
    std::atomic<bool>            _neuralA;
    std::atomic<bool>            _neuralB;
    std::atomic<bool>            bufferIsInit;
    std::atomic<int>             _ab;
    std::atomic<int>             _cd;

    inline Engine();
    inline ~Engine();

    inline void init(uint32_t rate, int32_t rt_prio_, int32_t rt_policy_);
    inline void clean_up();
    inline void do_work_mono();
    inline void process(uint32_t n_samples, float* output, float* output1);

private:
    ParallelThread               pro;
    ParallelThread               par;
    dcblocker::Dsp*              dcb;
    DenormalProtection           MXCSR;
    std::condition_variable      Sync;
    std::mutex                   WMutex;

    float*                       bufferoutput0;
    float*                       bufferoutput1;
    float*                       bufferinput0;
    float*                       _bufb;

    double                       fRec0[2];
    double                       fRec3[2];
    double                       fRec2[2];
    double                       fRec1[2];
    double                       fRec4[2];
    double                       fRec5[2];
    double                       fRec6[2];
    double                       fRec7[2];

    inline void processConv1();
    inline void processBuffer();
    inline void processSlotB(float* output);
    inline void processBufferedSlotB();
    inline void processDsp(uint32_t n_samples, float* output, float* output1);

    inline void setModel(NeuralModelLoader *slot,
                std::string *file, std::atomic<bool> *set);

    inline void setIRFile(ConvolverSelector *co, std::string *file);
};

inline Engine::Engine() :
    xrworker(),
    pro(),
    par(),
    dcb(dcblocker::plugin()),
    peq(eq::plugin()),
    ngate(noisegate::plugin()),
    slotA(&Sync),
    slotB(&Sync),
    conv(),
    conv1(),
    bufferoutput0(NULL),
    bufferoutput1(NULL),
    bufferinput0(NULL),
    _bufb(0) {
        bufsize = 0;
        buffersize = 0;
        phaseOffset = 0;
        bypass = 0;
        eqOnOff = 0;
        ngOnOff = 0;
        IRmode = 0;
        normSlotA = 0;
        normSlotB = 0;
        inputGain = 0.0;
        inputGain1 = 0.0;
        outputGain = 0.0;
        outputGain1 = 0.0;
        IRoutputGain = 0.0;
        IRoutputGain1 = 0.0;
        MasterOutGain = 0.0;
        IRmix = 0.0;
        buffered = 0.0;
        latency = 0.0;
        XrunCounter = 0.0;

        model_file = "None";
        model_file1 = "None";

        ir_file = "None";
        ir_file1 = "None";
        xrworker.start();
        pro.start();
        par.start();

        _neuralA.store(false, std::memory_order_release);
        _neuralB.store(false, std::memory_order_release);
};

inline Engine::~Engine(){
    xrworker.stop();
    pro.stop();
    par.stop();

    delete[] bufferoutput0;
    delete[] bufferoutput1;
    delete[] bufferinput0;

    dcb->del_instance(dcb);
    peq->del_instance(peq);
    ngate->del_instance(ngate);
    slotA.cleanUp();
    slotB.cleanUp();
    conv.stop_process();
    conv.cleanup();
    conv1.stop_process();
    conv1.cleanup();
};

inline void Engine::init(uint32_t rate, int32_t rt_prio_, int32_t rt_policy_) {
    s_rate = rate;
    dcb->init(rate);
    peq->init(rate);
    ngate->init(rate);
    slotA.init(rate);
    slotB.init(rate);

    rt_prio = rt_prio_;
    rt_policy = rt_policy_;

    _execute.store(false, std::memory_order_release);
    _notify_ui.store(false, std::memory_order_release);
    bufferIsInit.store(false, std::memory_order_release);
    _ab.store(0, std::memory_order_release);
    _cd.store(0, std::memory_order_release);

    xrworker.setThreadName("Worker");
    xrworker.set<Engine, &Engine::do_work_mono>(this);

    pro.setThreadName("RT-Parallel");
    pro.setPriority(rt_prio, rt_policy);
    pro.set<Engine, &Engine::processConv1>(this);

    par.setThreadName("RT-BUF");
    par.setPriority(rt_prio, rt_policy);
    par.set<0,Engine, &Engine::processBuffer>(this);
    par.set<1,Engine, &Engine::processBufferedSlotB>(this);

    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec0[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec3[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec2[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec1[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec4[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec5[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec6[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec7[l0] = 0.0;
};

void Engine::clean_up()
{
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec0[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec3[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec2[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec1[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec4[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec5[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec6[l0] = 0.0;
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec7[l0] = 0.0;
    // delete the internal DSP mem
}

inline void Engine::setModel(NeuralModelLoader *slot,
                std::string *file, std::atomic<bool> *set) {
    if ((*file).compare(slot->getModelFile()) != 0) {
        slot->setModelFile(*file);
        if (!slot->loadModel()) {
            *file = "None";
            set->store(false, std::memory_order_release);
        } else {
            set->store(true, std::memory_order_release);
        }
    }
}

inline void Engine::setIRFile(ConvolverSelector *co, std::string *file) {
    if (co->is_runnable()) {
        co->set_not_runnable();
        co->stop_process();
        std::unique_lock<std::mutex> lk(WMutex);
        Sync.wait_for(lk, std::chrono::milliseconds(160));
    }

    co->cleanup();
    co->set_samplerate(s_rate);
    co->set_buffersize(bufsize);

    if (*file != "None") {
        co->configure(*file, 1.0, 0, 0, 0, 0, 0);
        while (!co->checkstate());
        if(!co->start(rt_prio, rt_policy)) {
            *file = "None";
           // lv2_log_error(&logger,"impulse convolver update fail\n");
        }
    }
}

void Engine::do_work_mono() {
    // set neural models
    if (_ab.load(std::memory_order_acquire) == 1) {
        setModel(&slotA, &model_file, &_neuralA);
    } else if (_ab.load(std::memory_order_acquire) == 2) {
        setModel(&slotB, &model_file1, &_neuralB);
    } else if (_ab.load(std::memory_order_acquire) > 2) {
        setModel(&slotA, &model_file, &_neuralA);
        setModel(&slotB, &model_file1, &_neuralB);
    }
    // set ir files
    if (_cd.load(std::memory_order_acquire) == 1) {
        setIRFile(&conv, &ir_file);
    } else if (_cd.load(std::memory_order_acquire) == 2) {
        setIRFile(&conv1, &ir_file1);
    } else if (_cd.load(std::memory_order_acquire) > 2) {
        setIRFile(&conv, &ir_file);
        setIRFile(&conv1, &ir_file1);
    }
    
    // init buffer for background processing
    if (buffersize < bufsize) {
        buffersize = bufsize * 2;
        delete[] bufferoutput0;
        bufferoutput0 = NULL;
        bufferoutput0 = new float[buffersize];
        memset(bufferoutput0, 0, buffersize*sizeof(float));
        delete[] bufferoutput1;
        bufferoutput1 = NULL;
        bufferoutput1 = new float[buffersize];
        memset(bufferoutput1, 0, buffersize*sizeof(float));
        delete[] bufferinput0;
        bufferinput0 = NULL;
        bufferinput0 = new float[buffersize];
        memset(bufferinput0, 0, buffersize*sizeof(float));
        par.setTimeOut(std::max(100,static_cast<int>((bufsize/(s_rate*0.000001))*0.1)));
        bufferIsInit.store(true, std::memory_order_release);
        // set wait function time out for parallel processor thread
        pro.setTimeOut(std::max(100,static_cast<int>((bufsize/(s_rate*0.000001))*0.1)));
        slotA.setMaxBufferSize(bufsize * 2);
        slotB.setMaxBufferSize(bufsize * 2);
    }
    // set flag that work is done ready
    _execute.store(false, std::memory_order_release);
    // set flag that GUI need information about changed state
    _notify_ui.store(true, std::memory_order_release);
}

// process second convolver in parallel thread
inline void Engine::processConv1() {
    conv1.compute(bufsize, _bufb, _bufb);
}

// process dsp in buffered in a background thread
inline void Engine::processBuffer() {
    processDsp(bufsize, bufferoutput0, bufferoutput1);
}

// process slotB
inline void Engine::processSlotB(float* output) {
    double fSlow4 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(inputGain1));
    double fSlow2 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(outputGain1));
    // process input volume slot B
    if (_neuralB.load(std::memory_order_acquire)) {
        for (uint32_t i0 = 0; i0 < bufsize; i0 = i0 + 1) {
            fRec4[0] = fSlow4 + 0.999 * fRec4[1];
            output[i0] = float(double(output[i0]) * fRec4[0]);
            fRec4[1] = fRec4[0];
        }
    }

    // process slot B
    if (_neuralB.load(std::memory_order_acquire)) {
        slotB.compute(bufsize, output, output);
        if (normSlotB) slotB.normalize(bufsize, output);
    }

    // process output volume slot B
    if (_neuralB.load(std::memory_order_acquire)) {
        for (uint32_t i0 = 0; i0 < bufsize; i0 = i0 + 1) {
            fRec2[0] = fSlow2 + 0.999 * fRec2[1];
            output[i0] = float(double(output[i0]) * fRec2[0]);
            fRec2[1] = fRec2[0];
        }
    }
}

// process slotB in buffered in a background thread
inline void Engine::processBufferedSlotB() {
    processSlotB(bufferoutput0);
}

inline void Engine::processDsp(uint32_t n_samples, float* output, float* output1)
{
    if(n_samples<1) return;

    // basic bypass
    if (!bypass) {
        Sync.notify_all();
        return;
    }
    MXCSR.set_();

    // get controller values from host
    double fSlow0 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(inputGain));
    double fSlow3 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(outputGain));
    double fSlow1 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(IRoutputGain));
    double fSlow5 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(IRoutputGain1));

    // run noisegate
    if (ngOnOff)
        ngate->compute(n_samples, output);

    // process input volume slot A
    if (_neuralA.load(std::memory_order_acquire)) {
        for (uint32_t i0 = 0; i0 < n_samples; i0 = i0 + 1) {
            fRec0[0] = fSlow0 + 0.999 * fRec0[1];
            output[i0] = float(double(output[i0]) * fRec0[0]);
            fRec0[1] = fRec0[0];
        }
    }

    // process slot A
    if (_neuralA.load(std::memory_order_acquire)) {
        slotA.compute(n_samples, output, output);
        if (normSlotA) slotA.normalize(n_samples, output);
    }

    // process output volume slot A
    if (_neuralA.load(std::memory_order_acquire) ) {
        for (uint32_t i0 = 0; i0 < n_samples; i0 = i0 + 1) {
            fRec3[0] = fSlow3 + 0.999 * fRec3[1];
            output[i0] = float(double(output[i0]) * fRec3[0]);
            fRec3[1] = fRec3[0];
        }
    }

    // run eq
    if (eqOnOff)
        peq->compute(n_samples, output, output);

    if ((buffered == 1.0) && bufferIsInit.load(std::memory_order_acquire)) {
        // avoid buffer overflow on frame size change
        if (buffersize < n_samples) {
            bufsize = n_samples;
            bufferIsInit.store(false, std::memory_order_release);
            _execute.store(true, std::memory_order_release);
            xrworker.runProcess();
            return;
        }
        par.setProcessor(1);
        // get the buffer from previous process
        if (!par.processWait()) {
            XrunCounter += 1;
            _notify_ui.store(true, std::memory_order_release);
        }
        // copy incoming data to internal input buffer
        memcpy(bufferinput0, output, n_samples*sizeof(float));

        bufsize = n_samples;
        // copy processed data from last circle to output
        memcpy(output, bufferoutput0, bufsize*sizeof(float));
        // copy internal input buffer to process buffer for next circle
        memcpy(bufferoutput0, bufferinput0, bufsize*sizeof(float));

        // process data in background thread
        if (par.getProcess()) par.runProcess();
        else {
            XrunCounter += 1;
            _notify_ui.store(true, std::memory_order_release);
        }
        latency = n_samples;
    } else {
        processSlotB(output);
        if (!buffered) latency = 0.0;
    }

    // run dcblocker
    dcb->compute(n_samples, output, output);

    // set buffer for stereo output
    float bufa[n_samples];
    memcpy(bufa, output, n_samples*sizeof(float));
    float bufb[n_samples];
    memcpy(bufb, output, n_samples*sizeof(float));
    bufsize = n_samples;

    // process conv1 in parallel thread
    _bufb = bufb;
    if (!_execute.load(std::memory_order_acquire) && conv1.is_runnable()) {
        if (pro.getProcess()) {
            pro.runProcess();
        } else {
            XrunCounter += 1;
            _notify_ui.store(true, std::memory_order_release);
        }
    }

    // process conv
    if (!_execute.load(std::memory_order_acquire) && conv.is_runnable())
        conv.compute(n_samples, bufa, bufa);

    // wait for parallel processed conv1 when needed
    if (!_execute.load(std::memory_order_acquire) && conv1.is_runnable()) {
        if (!pro.processWait()) {
            XrunCounter += 1;
            _notify_ui.store(true, std::memory_order_release);
        }
    }

    if (IRmode == 0) { // Stereo mode
    // IRoutputGain 
        for (uint32_t i0 = 0; i0 < n_samples; i0 = i0 + 1) {
            fRec1[0] = fSlow1 + 0.999 * fRec1[1];
            output[i0] = bufa[i0] * fRec1[0];
            fRec1[1] = fRec1[0];
        }
        // IRoutputGain1
        for (uint32_t i0 = 0; i0 < n_samples; i0 = i0 + 1) {
            fRec5[0] = fSlow5 + 0.999 * fRec5[1];
            output1[i0] = bufb[i0] * fRec5[0];
            fRec5[1] = fRec5[0];
        }
    } else { // Mix mode
        // mix output when needed
        double fSlow7 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(MasterOutGain));
        if ((!_execute.load(std::memory_order_acquire) &&
                conv.is_runnable()) && conv1.is_runnable()) {
            double fSlow6 = 0.0010000000000000009 * double(IRmix);
            for (uint32_t i0 = 0; i0 < n_samples; i0 = i0 + 1) {
                fRec6[0] = fSlow6 + 0.999 * fRec6[1];
                output[i0] = bufa[i0] * (1.0 - fRec6[0]) + bufb[i0] * fRec6[0];
                fRec6[1] = fRec6[0];
            }
        } else if (!_execute.load(std::memory_order_acquire) && conv.is_runnable()) {
            memcpy(output, bufa, n_samples*sizeof(float));
        } else if (!_execute.load(std::memory_order_acquire) && conv1.is_runnable()) {
            memcpy(output, bufb, n_samples*sizeof(float));
        }
        // MasterOutGain
        for (uint32_t i0 = 0; i0 < n_samples; i0 = i0 + 1) {
            fRec7[0] = fSlow7 + 0.999 * fRec7[1];
            output[i0] *= fRec7[0];
            fRec7[1] = fRec7[0];
        }
        memcpy(output1, output, n_samples*sizeof(float));
    }

    // run noisegate
    if (ngOnOff)
        ngate->computeLeft(n_samples, output);


    // run noisegate
    if (ngOnOff)
        ngate->computeRight(n_samples, output1);
    
    // notify neural modeller that process cycle is done
    Sync.notify_all();
    MXCSR.reset_();
}

inline void Engine::process(uint32_t n_samples, float* output, float* output1) {
    // process in buffered mode
    if ((buffered > 1.0) && bufferIsInit.load(std::memory_order_acquire)) {
        // avoid buffer overflow on frame size change
        par.setProcessor(0);
        if (buffersize < n_samples) {
            bufsize = n_samples;
            bufferIsInit.store(false, std::memory_order_release);
            _execute.store(true, std::memory_order_release);
            xrworker.runProcess();
            return;
        }
        // get the buffer from previous process
        if (!par.processWait()) {
            XrunCounter += 1;
            _notify_ui.store(true, std::memory_order_release);
        }
        // copy incoming data to internal input buffer
        memcpy(bufferinput0, output, n_samples*sizeof(float));

        bufsize = n_samples;
        // copy processed data from last circle to output
        memcpy(output, bufferoutput0, bufsize*sizeof(float));
        memcpy(output1, bufferoutput1, bufsize*sizeof(float));
        // copy internal input buffer to process buffer for next circle
        memcpy(bufferoutput0, bufferinput0, bufsize*sizeof(float));
        memcpy(bufferoutput1, bufferinput0, bufsize*sizeof(float));

        // process data in background thread
        if (par.getProcess()) par.runProcess();
        else {
            XrunCounter += 1;
            _notify_ui.store(true, std::memory_order_release);
        }
        latency = n_samples;
    } else {
        // process latency free
        processDsp(n_samples, output, output1);
        if (!buffered) latency = 0.0;
    }
}

}; // end namespace neuralrack
#endif
