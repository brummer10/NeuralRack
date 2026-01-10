/*
 * Ratatouille.cpp
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */

#ifdef _WIN32
#define MINGW_STDTHREAD_REDUNDANCY_WARNING
#endif

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


#include "uris.h"

#include "engine.h"

#include "ParallelThread.h"


namespace neuralrack {

////////////////////////////// PLUG-IN CLASS ///////////////////////////

class Xneuralrack : public uris
{
private:
    Engine                       engine;

    int32_t                      rt_prio;
    int32_t                      rt_policy;
    float*                       input0;
    float*                       output0;
    float*                       output1;
    float*                       _inputGain;
    float*                       _inputGain1;
    float*                       _outputGain;
    float*                       _outputGain1;
    float*                       _IRoutputGain;
    float*                       _IRoutputGain1;
    float*                       _normA;
    float*                       _normB;
    uint32_t                     normA;
    uint32_t                     normB;
    float*                       _normSlotA;
    float*                       _normSlotB;
    float*                       _bypass;
    float*                       _eraseSlotA;
    float*                       _eraseSlotB;
    float*                       _eraseIr;
    float*                       _eraseIr1;
    float*                       _latency;
    float*                       _latencyms;
    float*                       _buffered;
    float*                       _xrun;
    float*                       _fVslider0;
    float*                       _fVslider1;
    float*                       _fVslider2;
    float*                       _fVslider3;
    float*                       _fVslider4;
    float*                       _fVslider5;
    float*                       _threshold;
    float*                       _eqOnOff;
    float*                       _ngOnOff;
    float*                       _IrMode;
    float*                       _IrMix;
    float*                       _MasterOutGain;
    uint32_t                     s_rate;
    double                       s_time;
    int                          processCounter;
    bool                         doit;

    std::atomic<bool>            _restore;

    // private functions
    inline void check_messages(uint32_t n_samples);
    inline void runBufferedDsp(uint32_t n_samples);
    inline void connect_(uint32_t port,void* data);
    inline void init_dsp_(uint32_t rate);
    inline void connect_all__ports(uint32_t port, void* data);
    inline void activate_f();
    inline void clean_up();
    inline void deactivate_f();

public:
    inline LV2_Atom* write_set_eqpos(LV2_Atom_Forge* forge,
                const LV2_URID urid, int value);
    inline LV2_Atom* write_set_file(LV2_Atom_Forge* forge,
                    const LV2_URID xlv2_model, const char* filename);
    inline const LV2_Atom* read_set_file(const LV2_Atom_Object* obj);
    inline void storeValue(LV2_State_Store_Function store, 
            LV2_State_Handle handle,const LV2_URID urid, float value);
    inline void storeFile(LV2_State_Store_Function store,
            LV2_State_Handle handle, const LV2_URID urid, const std::string file);
    const float* restoreValue(LV2_State_Retrieve_Function retrieve,
            LV2_State_Handle handle,LV2_URID urid);
    inline bool restoreFile(LV2_State_Retrieve_Function retrieve,
                LV2_State_Handle handle, const LV2_URID urid, std::string *file);
    // LV2 Descriptor
    static const LV2_Descriptor descriptor;
    static const void* extension_data(const char* uri);
    // static wrapper to private functions
    static void deactivate(LV2_Handle instance);
    static void cleanup(LV2_Handle instance);
    static void run(LV2_Handle instance, uint32_t n_samples);
    static void activate(LV2_Handle instance);
    static void connect_port(LV2_Handle instance, uint32_t port, void* data);

    static LV2_State_Status save_state(LV2_Handle instance,
                                       LV2_State_Store_Function store,
                                       LV2_State_Handle handle, uint32_t flags,
                                       const LV2_Feature* const* features);

    static LV2_State_Status restore_state(LV2_Handle instance,
                                          LV2_State_Retrieve_Function retrieve,
                                          LV2_State_Handle handle, uint32_t flags,
                                          const LV2_Feature* const*   features);

    static LV2_Handle instantiate(const LV2_Descriptor* descriptor,
                                double rate, const char* bundle_path,
                                const LV2_Feature* const* features);
  
    static LV2_Worker_Status work(LV2_Handle                 instance,
                                LV2_Worker_Respond_Function respond,
                                LV2_Worker_Respond_Handle   handle,
                                uint32_t size, const void*    data);
  
    static LV2_Worker_Status work_response(LV2_Handle  instance,
                                         uint32_t    size,
                                         const void* data);
    Xneuralrack();
    ~Xneuralrack();
};

// constructor
Xneuralrack::Xneuralrack() :
    engine(),
    rt_prio(0),
    rt_policy(0),
    input0(NULL),
    output0(NULL),
    output1(NULL),
    _inputGain(0),
    _inputGain1(0),
    _outputGain(0),
    _outputGain1(0),
    _IRoutputGain(0),
    _IRoutputGain1(0),
    _normA(0),
    _normB(0),
    _bypass(0),
    _eraseSlotA(0),
    _eraseSlotB(0),
    _eraseIr(0),
    _eraseIr1(0),
    _latency(0),
    _latencyms(0),
    _buffered(0),
    _xrun(0),
    _fVslider0(0),
    _fVslider1(0),
    _fVslider2(0),
    _fVslider3(0),
    _fVslider4(0),
    _fVslider5(0),
    _threshold(0),
    _eqOnOff(0),
    _ngOnOff(0),
    _IrMode(0),
    _IrMix(0),
    _MasterOutGain(0) {
        map = nullptr;
        schedule = nullptr;
        control = nullptr;
        notify = nullptr;
        log = nullptr;
        memset(&logger,0,sizeof(logger));
};

// destructor
Xneuralrack::~Xneuralrack() {
};

///////////////////////// PRIVATE CLASS  FUNCTIONS /////////////////////

void Xneuralrack::init_dsp_(uint32_t rate)
{
    s_rate = rate;
    s_time = (1.0 / (double)s_rate) * 1000;

    if (!rt_policy) rt_policy = 1; //SCHED_FIFO;
    engine.init(rate, rt_prio, rt_policy);

    processCounter = 0;
    doit = false;
    _restore.store(false, std::memory_order_release);
}

// connect the Ports used by the plug-in class
void Xneuralrack::connect_(uint32_t port,void* data)
{
    switch (port)
    {
        case 0:
            input0 = static_cast<float*>(data);
            break;
        case 1:
            output0 = static_cast<float*>(data);
            break;
        case 2:
            _inputGain = static_cast<float*>(data);
            break;
        case 3:
            _outputGain = static_cast<float*>(data);
            break;
        case 4:
            _outputGain1 = static_cast<float*>(data);
            break;
        case 5:
            control = (const LV2_Atom_Sequence*)data;
            break;
        case 6:
            notify = (LV2_Atom_Sequence*)data;
            break;
        case 7:
            _IRoutputGain = static_cast<float*>(data);
            break;
        case 8: 
            _IRoutputGain1 = static_cast<float*>(data); 
            break;
        case 9:
            _normA = static_cast<float*>(data);
            break;
        case 10:
            _normB = static_cast<float*>(data);
            break;
        case 11:
            _inputGain1 = static_cast<float*>(data);
            break;
        case 12:
            _normSlotA = static_cast<float*>(data);
            break;
        case 13:
            _normSlotB = static_cast<float*>(data);
            break;
        case 14:
            _bypass = static_cast<float*>(data);
            break;
        case 15:
            _eraseSlotA = static_cast<float*>(data);
            break;
        case 16:
            _eraseSlotB = static_cast<float*>(data);
            break;
        case 17:
            _eraseIr = static_cast<float*>(data);
            break;
        case 18:
            _eraseIr1 = static_cast<float*>(data);
            break;
        case 19:
            _latency = static_cast<float*>(data);
            break;
        case 20:
            _buffered = static_cast<float*>(data);
            break;
        case 21:
            output1 = static_cast<float*>(data);
            break;
        case 22:
            _latencyms = static_cast<float*>(data);
            break;
        case 23:
            _xrun = static_cast<float*>(data);
            break;
        case 24: 
            _fVslider1 = static_cast<float*>(data); 
            break;
        case 25: 
            _fVslider0 = static_cast<float*>(data); 
            break;
        case 26: 
            _fVslider2 = static_cast<float*>(data);
            break;
        case 27: 
            _fVslider3 = static_cast<float*>(data);
            break;
        case 28: 
            _fVslider4 = static_cast<float*>(data);
            break;
        case 29: 
            _fVslider5 = static_cast<float*>(data); 
            break;
        case 30:
            _eqOnOff = static_cast<float*>(data);
            break;
        case 31: 
            _threshold = static_cast<float*>(data); 
            break;
        case 32:
            _ngOnOff = static_cast<float*>(data);
            break;
        case 33:
            _IrMode = static_cast<float*>(data);
            break;
        case 34:
            _IrMix = static_cast<float*>(data);
            break;
        case 35:
            _MasterOutGain = static_cast<float*>(data);
            break;
        default:
            break;
    }
}

void Xneuralrack::activate_f()
{
    // allocate the internal DSP mem
}

void Xneuralrack::clean_up()
{
    engine.clean_up();
    // delete the internal DSP mem
}

void Xneuralrack::deactivate_f()
{
    // delete the internal DSP mem
}

// prepare atom message with int value
inline LV2_Atom* Xneuralrack::write_set_eqpos(LV2_Atom_Forge* forge,
                                    const LV2_URID urid, int value) {
    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(forge, 0);
    LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_object(
                        forge, &frame, 1, patch_Set);
    lv2_atom_forge_key(forge, patch_property);
    lv2_atom_forge_urid(forge, urid);
    lv2_atom_forge_key(forge, patch_value);
    lv2_atom_forge_int(forge, value);
    lv2_atom_forge_pop(forge, &frame);
    return set;
}

// prepare atom message with file path
inline LV2_Atom* Xneuralrack::write_set_file(LV2_Atom_Forge* forge,
                    const LV2_URID xlv2_model, const char* filename) {

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(forge, 0);
    LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_object(
                        forge, &frame, 1, patch_Set);

    lv2_atom_forge_key(forge, patch_property);
    lv2_atom_forge_urid(forge, xlv2_model);
    lv2_atom_forge_key(forge, patch_value);
    lv2_atom_forge_path(forge, filename, strlen(filename) + 1);

    lv2_atom_forge_pop(forge, &frame);
    return set;
}

// read atom message with file path
inline const LV2_Atom* Xneuralrack::read_set_file(const LV2_Atom_Object* obj) {
    if (obj->body.otype != patch_Set) {
        return NULL;
    }

    const LV2_Atom* property = NULL;
    lv2_atom_object_get(obj, patch_property, &property, 0);

    if (property && (property->type == atom_URID)) {
        if (((LV2_Atom_URID*)property)->body == xlv2_model_file)
            engine._ab.store(1, std::memory_order_release);
        else if (((LV2_Atom_URID*)property)->body == xlv2_model_file1)
            engine._ab.store(2, std::memory_order_release);
        else if (((LV2_Atom_URID*)property)->body == xlv2_ir_file)
            engine._cd.store(1, std::memory_order_release);
        else if (((LV2_Atom_URID*)property)->body == xlv2_ir_file1)
            engine._cd.store(2, std::memory_order_release);
        else return NULL;
    }

    const LV2_Atom* file_path = NULL;
    lv2_atom_object_get(obj, patch_value, &file_path, 0);
    if (!file_path || (file_path->type != atom_Path)) {
        return NULL;
    }

    return file_path;
}

// read all incoming atom messages
inline void Xneuralrack::check_messages(uint32_t n_samples)
{
    if(n_samples<1) return;
    const uint32_t notify_capacity = this->notify->atom.size;
    lv2_atom_forge_set_buffer(&forge, (uint8_t*)notify, notify_capacity);
    lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);

    engine.bufsize = n_samples;

    LV2_ATOM_SEQUENCE_FOREACH(control, ev) {
        if (lv2_atom_forge_is_object_type(&forge, ev->body.type)) {
            const LV2_Atom_Object* obj = (LV2_Atom_Object*)&ev->body;
            if (obj->body.otype == patch_Get) {
                if (engine.model_file != "None")
                    write_set_file(&forge, xlv2_model_file, engine.model_file.data());
                if (engine.model_file1 != "None")
                    write_set_file(&forge, xlv2_model_file1, engine.model_file1.data());
                if (engine.ir_file != "None")
                    write_set_file(&forge, xlv2_ir_file, engine.ir_file.data());
                if (engine.ir_file1 != "None")
                    write_set_file(&forge, xlv2_ir_file1, engine.ir_file1.data());
           } else if (obj->body.otype == patch_Set) {
                const LV2_Atom* file_path = read_set_file(obj);
                if (file_path) {
                    if (engine._ab.load(std::memory_order_acquire) == 1)
                        engine.model_file = (const char*)(file_path+1);
                    else if (engine._ab.load(std::memory_order_acquire) == 2)
                        engine.model_file1 = (const char*)(file_path+1);
                    else if (engine._cd.load(std::memory_order_acquire) == 1)
                        engine.ir_file = (const char*)(file_path+1);
                    else if (engine._cd.load(std::memory_order_acquire) == 2)
                        engine.ir_file1 = (const char*)(file_path+1);
                    if (!doit) doit = true;
                } else {
                    const LV2_Atom* property = NULL;
                    lv2_atom_object_get(obj, patch_property, &property, 0);

                    if (property && (property->type == atom_URID)) {
                        if (((LV2_Atom_URID*)property)->body == xlv2_eq_pos) {
                            const LV2_Atom* value = NULL;
                            lv2_atom_object_get(obj, patch_value, &value, 0);
                            if (value && (value->type == atom_Int)) {
                                int* eqpos = (int*)LV2_ATOM_BODY(value);
                                engine.setEQPos((*eqpos));
                            }
                        }
                    }
                }
            }
        }
    }

    // fetch parameters from host
    engine.normSlotA = static_cast<int32_t>(*_normSlotA);
    engine.normSlotB = static_cast<int32_t>(*_normSlotB);
    engine.bypass = static_cast<uint32_t>(*_bypass);
    engine.eqOnOff = static_cast<uint32_t>(*_eqOnOff);
    engine.ngOnOff = static_cast<uint32_t>(*_ngOnOff);
    engine.IRmode = static_cast<uint32_t>(*_IrMode);
    engine.inputGain = *_inputGain;
    engine.inputGain1 = *_inputGain1;
    engine.outputGain = *_outputGain;
    engine.outputGain1 = *_outputGain1;
    engine.IRoutputGain = *_IRoutputGain;
    engine.IRoutputGain1 = *_IRoutputGain1;
    engine.buffered = *_buffered;
    engine.peq->fVslider0 = *_fVslider0;
    engine.peq->fVslider1 = *_fVslider1;
    engine.peq->fVslider2 = *_fVslider2;
    engine.peq->fVslider3 = *_fVslider3;
    engine.peq->fVslider4 = *_fVslider4;
    engine.peq->fVslider5 = *_fVslider5;
    engine.ngate->threshold = *_threshold;
    engine.IRmix = *_IrMix;
    engine.MasterOutGain = *_MasterOutGain;

    // check if a model or IR file is to be removed
    if ((*_eraseSlotA)) {
        engine._ab.fetch_add(1, std::memory_order_relaxed);
        engine.model_file = "None";
        if (!doit) doit = true;
        (*_eraseSlotA) = 0.0;
    } else if ((*_eraseSlotB)) {
        engine._ab.fetch_add(2, std::memory_order_relaxed);
        engine.model_file1 = "None";
        if (!doit) doit = true;
        (*_eraseSlotB) = 0.0;
    } else if ((*_eraseIr)) {
        engine._cd.fetch_add(1, std::memory_order_relaxed);
        engine.ir_file = "None";
        if (!doit) doit = true;
        (*_eraseIr) = 0.0;
    } else if ((*_eraseIr1)) {
        engine._cd.fetch_add(2, std::memory_order_relaxed);
        engine.ir_file1 = "None";
        if (!doit) doit = true;
        (*_eraseIr1) = 0.0;
    }

    if (_restore.load(std::memory_order_acquire)) {
        if (!doit) doit = true;
        _restore.store(false, std::memory_order_release);
    }
    // check if normalisation is pressed for conv
    if (normA != static_cast<uint32_t>(*(_normA))) {
        normA = static_cast<uint32_t>(*(_normA));
        engine._cd.fetch_add(1, std::memory_order_relaxed);
        engine.conv.set_normalisation(normA);
        if (engine.ir_file.compare("None") != 0) {
            if (!doit) doit = true;
        }
    }
    // check if normalisation is pressed for conv1
    if (normB != static_cast<uint32_t>(*(_normB))) {
        normB = static_cast<uint32_t>(*(_normB));
        engine._cd.fetch_add(2, std::memory_order_relaxed);
        engine.conv1.set_normalisation(normB);
        if (engine.ir_file1.compare("None") != 0) {
            if (!doit) doit = true;
        }
    }
    // init buffer for background processing when needed
    if (!engine.bufferIsInit.load(std::memory_order_acquire)) {
        if (!doit) doit = true;
    }
    // run worker thread when needed
    if (doit && !engine._execute.load(std::memory_order_acquire)) {
        engine._execute.store(true, std::memory_order_release);
        engine.xrworker.runProcess();
        doit = false;
    } 
    // notify UI on changed model files
    if (engine._notify_ui.load(std::memory_order_acquire)) {
        engine._notify_ui.store(false, std::memory_order_release);

        write_set_file(&forge, xlv2_model_file, engine.model_file.data());
        write_set_file(&forge, xlv2_model_file1, engine.model_file1.data());

        write_set_file(&forge, xlv2_ir_file, engine.ir_file.data());
        write_set_file(&forge, xlv2_ir_file1, engine.ir_file1.data());
        engine._ab.store(0, std::memory_order_release);
        engine._cd.store(0, std::memory_order_release);
        write_set_eqpos(&forge, xlv2_eq_pos, engine.eqPos);
    }
}

inline void Xneuralrack::runBufferedDsp(uint32_t n_samples)
{
    // nothing to do for zero samples
    if(n_samples<1) return;

    // copy input to output when they are not the same buffers
    // doing in place processing
    if(output0 != input0)
        memcpy(output0, input0, n_samples*sizeof(float));
    if(output1 != input0)
        memcpy(output1, input0, n_samples*sizeof(float));

    // the early bird die
    if (processCounter < 5) {
        processCounter++;
        return;
    }
    // check atom messages (full cycle)
    check_messages(n_samples);
    // run engine
    engine.process(n_samples, output0, output1);
    // report lanency
    *(_latency) = engine.latency;
    *(_latencyms) = engine.latency * s_time;
    *(_xrun) = engine.XrunCounter;
}

void Xneuralrack::connect_all__ports(uint32_t port, void* data)
{
    // connect the Ports used by the plug-in class
    connect_(port,data);
}


void Xneuralrack::storeValue(LV2_State_Store_Function store, 
            LV2_State_Handle handle,const LV2_URID urid, float value) {
    const int rw = value;
    store(handle,urid,&rw, sizeof(rw),
          atom_Int, LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
}

// write file path to state
inline void Xneuralrack::storeFile(LV2_State_Store_Function store,
            LV2_State_Handle handle, const LV2_URID urid, const std::string file) {

    store(handle, urid, file.data(), strlen(file.data()) + 1,
          atom_String, LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
}


const float* Xneuralrack::restoreValue(LV2_State_Retrieve_Function retrieve,
            LV2_State_Handle handle,LV2_URID urid) {
    size_t      size;
    uint32_t    type;
    uint32_t    fflags;

    const void* value = retrieve(handle, urid, &size, &type, &fflags);
    if (value) return  ((const float *)value);
    return NULL;
}


// retrieve file path from state
inline bool Xneuralrack::restoreFile(LV2_State_Retrieve_Function retrieve,
                LV2_State_Handle handle, const LV2_URID urid, std::string *file) {

    size_t      size;
    uint32_t    type;
    uint32_t    fflags;
    const void* name = retrieve(handle, urid, &size, &type, &fflags);
    if (name) {
        *file = (const char*)(name);
        return (!(*file).empty() && ((*file) != "None"));
    }
    return false;
}

////////////////////// STATIC CLASS  FUNCTIONS  ////////////////////////

LV2_State_Status Xneuralrack::save_state(LV2_Handle instance,
                                     LV2_State_Store_Function store,
                                     LV2_State_Handle handle, uint32_t flags,
                                     const LV2_Feature* const* features) {

    Xneuralrack* self = static_cast<Xneuralrack*>(instance);

    self->storeFile(store, handle, self->xlv2_model_file, self->engine.model_file);
    self->storeFile(store, handle, self->xlv2_model_file1, self->engine.model_file1);
    self->storeFile(store, handle, self->xlv2_ir_file, self->engine.ir_file);
    self->storeFile(store, handle, self->xlv2_ir_file1, self->engine.ir_file1);
    self->storeValue(store, handle, self->xlv2_eq_pos, self->engine.eqPos);

    return LV2_STATE_SUCCESS;
}

LV2_State_Status Xneuralrack::restore_state(LV2_Handle instance,
                                        LV2_State_Retrieve_Function retrieve,
                                        LV2_State_Handle handle, uint32_t flags,
                                        const LV2_Feature* const*   features) {

    Xneuralrack* self = static_cast<Xneuralrack*>(instance);

    if (self->restoreFile(retrieve, handle, self->xlv2_model_file, &self->engine.model_file))
        self->engine._ab.fetch_add(1, std::memory_order_relaxed);
    if (self->restoreFile(retrieve, handle, self->xlv2_model_file1, &self->engine.model_file1))
        self->engine._ab.fetch_add(2, std::memory_order_relaxed);
    if (self->restoreFile(retrieve, handle, self->xlv2_ir_file, &self->engine.ir_file))
        self->engine._cd.fetch_add(1, std::memory_order_relaxed);
    if (self->restoreFile(retrieve, handle, self->xlv2_ir_file1, &self->engine.ir_file1))
        self->engine._cd.fetch_add(2, std::memory_order_relaxed);

    float* value = NULL;
    value = (float *)self->restoreValue(retrieve,handle, self->xlv2_eq_pos);
    if (value) {
        if (*((int *)value) != self->engine.eqPos) {
            self->engine.eqPos =  *((int *)value);
            self->write_set_eqpos(&self->forge, self->xlv2_eq_pos, self->engine.eqPos);
        }
    }

    self-> _restore.store(true, std::memory_order_release);
    return LV2_STATE_SUCCESS;
}

LV2_Handle 
Xneuralrack::instantiate(const LV2_Descriptor* descriptor,
                            double rate, const char* bundle_path,
                            const LV2_Feature* const* features)
{
    // init the plug-in class
    Xneuralrack *self = new Xneuralrack();
    if (!self) {
        return NULL;
    }

    const LV2_Options_Option* options  = NULL;
    uint32_t bufsize = 0;

    for (int32_t i = 0; features[i]; ++i) {
        if (!strcmp(features[i]->URI, LV2_URID__map)) {
            self->map = (LV2_URID_Map*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_WORKER__schedule)) {
            self->schedule = (LV2_Worker_Schedule*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_OPTIONS__options)) {
            options = (const LV2_Options_Option*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_LOG__log)) {
            self->log = (LV2_Log_Log*)features[i]->data;
        }
    }

    if (!self->map) {
        fprintf(stderr,"Missing required feature uri:map.\n EXIT here!!\n");
        cleanup((LV2_Handle)self);
        return nullptr;
    } else {
        self->map_uris(self->map);
    }

    if (self->log) {
        lv2_log_logger_init(&self->logger, self->map, self->log);
    }

    if (!self->schedule) {
        lv2_log_error(&self->logger, "Missing feature work:schedule.\n");
    }

    if (!options) {
        lv2_log_error(&self->logger, "Missing feature options.\n");
    }
    else {
        LV2_URID bufsz_max = self->map->map(self->map->handle, LV2_BUF_SIZE__maxBlockLength);
        LV2_URID bufsz_    = self->map->map(self->map->handle,"http://lv2plug.in/ns/ext/buf-size#nominalBlockLength");
        LV2_URID atom_Int = self->map->map(self->map->handle, LV2_ATOM__Int);
        LV2_URID tshed_pol = self->map->map (self->map->handle, "http://ardour.org/lv2/threads/#schedPolicy");
        LV2_URID tshed_pri = self->map->map (self->map->handle, "http://ardour.org/lv2/threads/#schedPriority");

        for (const LV2_Options_Option* o = options; o->key; ++o) {
            if (o->context == LV2_OPTIONS_INSTANCE &&
              o->key == bufsz_ && o->type == atom_Int) {
                bufsize = *(const int32_t*)o->value;
            } else if (o->context == LV2_OPTIONS_INSTANCE &&
              o->key == bufsz_max && o->type == atom_Int) {
                if (!bufsize)
                    bufsize = *(const int32_t*)o->value;
            } else if (o->context == LV2_OPTIONS_INSTANCE &&
                o->key == tshed_pol && o->type == atom_Int) {
                self->rt_policy = *(const int32_t*)o->value;
            } else if (o->context == LV2_OPTIONS_INSTANCE &&
                o->key == tshed_pri && o->type == atom_Int) {
                self->rt_prio = *(const int32_t*)o->value;
            }
        }

        if (bufsize == 0) {
            lv2_log_error(&self->logger, "No maximum buffer size given.\n");
        } else {
            self->engine.bufsize = bufsize;
            lv2_log_note(&self->logger, "using block size: %d\n", bufsize);
        }
    }

    self->map_uris(self->map);
    lv2_atom_forge_init(&self->forge, self->map);
    self->init_dsp_((uint32_t)rate);

    return (LV2_Handle)self;
}

void Xneuralrack::connect_port(LV2_Handle instance, 
                                   uint32_t port, void* data)
{
    // connect all ports
    static_cast<Xneuralrack*>(instance)->connect_all__ports(port, data);
}

void Xneuralrack::activate(LV2_Handle instance)
{
    // allocate needed mem
    static_cast<Xneuralrack*>(instance)->activate_f();
}

void Xneuralrack::run(LV2_Handle instance, uint32_t n_samples)
{
    // run dsp
    static_cast<Xneuralrack*>(instance)->runBufferedDsp(n_samples);
}

void Xneuralrack::deactivate(LV2_Handle instance)
{
    // free allocated mem
    static_cast<Xneuralrack*>(instance)->deactivate_f();
}

void Xneuralrack::cleanup(LV2_Handle instance)
{
    // well, clean up after us
    Xneuralrack* self = static_cast<Xneuralrack*>(instance);
    self->clean_up();
    delete self;
}

LV2_Worker_Status Xneuralrack::work(LV2_Handle instance,
     LV2_Worker_Respond_Function respond,
     LV2_Worker_Respond_Handle   handle,
     uint32_t                    size,
     const void*                 data)
{
  static_cast<Xneuralrack*>(instance)->engine.do_work_mono();
  return LV2_WORKER_SUCCESS;
}

LV2_Worker_Status Xneuralrack::work_response(LV2_Handle instance,
              uint32_t    size,
              const void* data)
{
  //printf("worker respose.\n");
  return LV2_WORKER_SUCCESS;
}

const void* Xneuralrack::extension_data(const char* uri)
{
    static const LV2_Worker_Interface worker = { work, work_response, NULL };
    static const LV2_State_Interface  state  = { save_state, restore_state };

    if (!strcmp(uri, LV2_WORKER__interface)) {
        return &worker;
    }
    else if (!strcmp(uri, LV2_STATE__interface)) {
        return &state;
    }

    return NULL;
}

const LV2_Descriptor Xneuralrack::descriptor =
{
    PLUGIN_URI ,
    Xneuralrack::instantiate,
    Xneuralrack::connect_port,
    Xneuralrack::activate,
    Xneuralrack::run,
    Xneuralrack::deactivate,
    Xneuralrack::cleanup,
    Xneuralrack::extension_data
};

} // end namespace neuralrack

////////////////////////// LV2 SYMBOL EXPORT ///////////////////////////

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
    switch (index)
    {
        case 0:
            return &neuralrack::Xneuralrack::descriptor;
        default:
            return NULL;
    }
}

///////////////////////////// FIN //////////////////////////////////////
