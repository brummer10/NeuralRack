/*
 * fftconvolver.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */


#pragma once

#ifndef FFTCONVOLVER_H_
#define FFTCONVOLVER_H_

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

#include <stdint.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <sndfile.hh>

#include "TwoStageFFTConvolver.h"
#include "ParallelThread.h"
#include "gx_resampler.h"


/****************************************************************
 ** Audiofile - class to handle audio file read
 */

class Audiofile {
public:

    enum {
        TYPE_OTHER,
        TYPE_CAF,
        TYPE_WAV,
        TYPE_AIFF,
        TYPE_AMB
    };

    enum {
        FORM_OTHER,
        FORM_16BIT,
        FORM_24BIT,
        FORM_32BIT,
        FORM_FLOAT
    };

    enum {
        ERR_NONE    = 0,
        ERR_MODE    = -1,
        ERR_TYPE    = -2,
        ERR_FORM    = -3,
        ERR_OPEN    = -4,
        ERR_SEEK    = -5,
        ERR_DATA    = -6,
        ERR_READ    = -7,
        ERR_WRITE   = -8
    };

    Audiofile(void);
    ~Audiofile(void);

    int type(void) const      { return _type; }
    int form(void) const      { return _form; }
    int rate(void) const      { return _rate; }
    int chan(void) const      { return _chan; }
    unsigned int size(void) const { return _size; }

    int open_read(std::string name);
    int close(void);

    int seek(unsigned int posit);
    int read(float *data, unsigned int frames);

private:

    void reset(void);

    SNDFILE     *_sndfile;
    int          _type;
    int          _form;
    int          _rate;
    int          _chan;
    unsigned int _size;
};

/****************************************************************
 ** ConvolverBase - virtual base class to select the convolver to use
 */

class ConvolverBase
{
public:
    virtual bool start(int32_t policy, int32_t priority) {return true;}
    virtual void set_normalisation(uint32_t norm) {}
    virtual uint32_t get_normalisation() { return 0;}
    virtual bool configure(std::string fname, float gain, unsigned int delay,
                            unsigned int offset, unsigned int length,
                            unsigned int size, unsigned int bufsize) {return false;}
    virtual inline std::string getIrFile() { return "";}
    virtual void compute(int32_t count, float* input, float *output) {}
    virtual bool checkstate() { return true;}
    virtual inline void set_not_runnable() {}
    virtual inline bool is_runnable() { return false;}
    virtual inline void set_buffersize(uint32_t sz) {}
    virtual void set_samplerate(uint32_t sr) {}
    virtual int stop_process() {return 0;}
    virtual int cleanup() {return 0;}

    ConvolverBase() {};
    virtual ~ConvolverBase() {};
};

/****************************************************************
 ** DoubleThreadConvolver - convolver for larger IR files, using a background thread to handle the tail
 */

class DoubleThreadConvolver: public ConvolverBase, public fftconvolver::TwoStageFFTConvolver
{
public:
    std::mutex mo;
    std::condition_variable co;
    bool start(int32_t policy, int32_t priority) override {
        if (!pro.isRunning()) {
            pro.start(); 
            pro.setThreadName("Convolver");
            pro.setPriority(25, 1); //SCHED_FIFO
            pro.setTimeOut(200);
            pro.set<DoubleThreadConvolver, &DoubleThreadConvolver::backgroundProcessing>(this);
        }
        return ready;}

    void set_normalisation(uint32_t norm) override;

    uint32_t get_normalisation() override { return norm;}

    bool configure(std::string fname, float gain, unsigned int delay, unsigned int offset,
                    unsigned int length, unsigned int size, unsigned int bufsize) override;

    inline std::string getIrFile() override;

    void compute(int32_t count, float* input, float *output) override;

    bool checkstate() override { return true;}

    inline void set_not_runnable() override { ready = false;}

    inline bool is_runnable() override { return ready;}

    inline void set_buffersize(uint32_t sz) override { buffersize = sz;}

    inline void set_samplerate(uint32_t sr) override { samplerate = sr;}

    int stop_process() override {
            ready = false;
            return 0;}

    int cleanup () override {
            reset();
            return 0;}

    DoubleThreadConvolver()
        : resamp(), ready(false), samplerate(0), pro() {
            norm = 0;}

    ~DoubleThreadConvolver() { reset(); pro.stop();}

protected:
    void startBackgroundProcessing() override;
    void waitForBackgroundProcessing() override;

private:
    friend class ParallelThread;
    gx_resample::BufferResampler resamp;
    void backgroundProcessing() { return doBackgroundProcessing();}
    volatile bool ready;
    uint32_t buffersize;
    uint32_t samplerate;
    uint32_t norm;
    std::string filename;
    ParallelThread pro;
    std::atomic<bool> setWait;
    bool get_buffer(std::string fname, float **buffer, uint32_t* rate, int* size);
    void normalize(float* buffer, int asize);
};

/****************************************************************
 ** SingleThreadConvolver - convolver for small IR files, process in a single thread
 */

class SingleThreadConvolver: public ConvolverBase, public fftconvolver::FFTConvolver
{
public:
    bool start(int32_t policy, int32_t priority) override {
        return ready;}

    void set_normalisation(uint32_t norm) override;

    uint32_t get_normalisation() override { return norm;}

    bool configure(std::string fname, float gain, unsigned int delay, unsigned int offset,
                    unsigned int length, unsigned int size, unsigned int bufsize) override;

    inline std::string getIrFile() override;

    void compute(int32_t count, float* input, float *output) override;

    bool checkstate() override { return true;}

    inline void set_not_runnable() override { ready = false;}

    inline bool is_runnable() override { return ready;}

    inline void set_buffersize(uint32_t sz) override { buffersize = sz;}

    inline void set_samplerate(uint32_t sr) override { samplerate = sr;}

    int stop_process() override {
            ready = false;
            return 0;}

    int cleanup () override {
            reset();
            return 0;}

    SingleThreadConvolver()
        : resamp(), ready(false), samplerate(0) { norm = 0;}

    ~SingleThreadConvolver() { reset();}

private:
    gx_resample::BufferResampler resamp;
    volatile bool ready;
    uint32_t buffersize;
    uint32_t samplerate;
    uint32_t norm;
    std::string filename;
    bool get_buffer(std::string fname, float **buffer, uint32_t* rate, int* size);
    void normalize(float* buffer, int asize);
};

/****************************************************************
 ** ConvolverSelector - class to select the convolver to use based on the file size
 */

class ConvolverSelector
{
public:
    bool start(int32_t policy, int32_t priority) {
            return conv->start(policy, priority);}

    void set_normalisation(uint32_t norm) {
            sconv.set_normalisation(norm);
            dconv.set_normalisation(norm);}

    uint32_t get_normalisation() { 
        return conv->get_normalisation();
    }

    bool configure(std::string fname, float gain, unsigned int delay,
                            unsigned int offset, unsigned int length,
                            unsigned int size, unsigned int bufsize);

    inline std::string getIrFile() {
        return conv->getIrFile();
    }

    void compute(int32_t count, float* input, float *output) {
            conv->compute(count, input, output);}

    bool checkstate() {
            return conv->checkstate();}

    inline void set_not_runnable() {
            conv->set_not_runnable();}

    inline bool is_runnable() {
            return conv->is_runnable();}

    inline void set_buffersize(uint32_t sz) {
            sconv.set_buffersize(sz);
            dconv.set_buffersize(sz);}

    void set_samplerate(uint32_t sr) {
            sconv.set_samplerate(sr);
            dconv.set_samplerate(sr);}

    int stop_process() {
            return conv->stop_process();}

    int cleanup() {
            return conv->cleanup();}

    ConvolverSelector():
            sconv(),
            dconv(){        
            dconv.start(25, 1);
            conv = &sconv;
            }

    ~ ConvolverSelector() {}
    
private:
    ConvolverBase *conv;
    SingleThreadConvolver sconv;
    DoubleThreadConvolver dconv;
};

#endif  // FFTCONVOLVER_H_
