/*
 * NeuralRackvst.cpp
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */

#include "vestige.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <stdint.h>
#include <stddef.h>
#include "NeuralRack.cc"

typedef struct ERect {
    short top;
    short left;
    short bottom;
    short right;
} ERect;

#define PLUGIN_UID 'Nrbr'

#define WINDOW_WIDTH  620
#define WINDOW_HEIGHT 580

#define FlagsChunks (1 << 5)

struct neuralrack_plugin_t {
    AEffect* effect;
    NeuralRack *r;
    ERect editorRect;
    int width, height;
    float SampleRate;
    std::string state;
};

/****************************************************************
 ** connect value change messages from the GUI to the engine
 */

// send value changes from GUI to the engine
void sendValueChanged(X11_UI *ui, int port, float value) {
    NeuralRack *r = (NeuralRack*)ui->win->private_struct;
    r->sendValueChanged(port, value);
}

// send a file name from GUI to the engine
void sendFileName(X11_UI *ui, ModelPicker* m, int old){
    NeuralRack *r = (NeuralRack*)ui->win->private_struct;
    r->sendFileName(m, old);
}

// Forward declarations
static intptr_t dispatcher(AEffect*, int32_t, int32_t, intptr_t, void*, float);
static void processReplacing(AEffect*, float**, float**, int32_t);
static void setParameter(AEffect*, int32_t, float);
static float getParameter(AEffect*, int32_t);

// --- Param helpers ---
static void getParameterName(AEffect*, int32_t, char*);

// --- Main Entry ---

extern "C" __attribute__ ((visibility ("default")))
AEffect* VSTPluginMain(audioMasterCallback audioMaster) {
    neuralrack_plugin_t* plug = (neuralrack_plugin_t*)calloc(1, sizeof(neuralrack_plugin_t));
    AEffect* effect = (AEffect*)calloc(1, sizeof(AEffect));
    plug->r = new NeuralRack();
    effect->object = plug;
    plug->effect = effect;
    plug->width = WINDOW_WIDTH;
    plug->height = WINDOW_HEIGHT;
    plug->editorRect = {0, 0, (short) plug->height, (short) plug->width};
    plug->SampleRate = 48000.0;

    effect->magic = kEffectMagic;
    effect->dispatcher = dispatcher;
    effect->processReplacing = processReplacing;
    effect->setParameter = setParameter;
    effect->getParameter = getParameter;
    effect->numPrograms = 1;
    effect->numParams = 0;
    effect->numInputs = 1;
    effect->numOutputs = 2;
    effect->flags = effFlagsHasEditor | effFlagsCanReplacing | FlagsChunks;
    effect->uniqueID = PLUGIN_UID;
    return effect;
}

// --- Audio processing ---
static void processReplacing(AEffect* effect, float** inputs, float** outputs, int32_t sampleFrames) {
    neuralrack_plugin_t* plug = (neuralrack_plugin_t*)effect->object;

    float* input = inputs[0];
    float* left_output = outputs[0];
    float* right_output = outputs[1];
    if(left_output != input)
        memcpy(left_output, input, sampleFrames*sizeof(float));
    if(right_output != input)
        memcpy(right_output, input, sampleFrames*sizeof(float));
    
    plug->r->process(sampleFrames, left_output, right_output);
}

// --- Parameter handling ---
static void setParameter(AEffect* effect, int32_t index, float value) {
}

static float getParameter(AEffect* effect, int32_t index) {
    return 0.0;
}

static void getParameterName(AEffect*, int32_t index, char* label) {
}

// --- state handling ---
void saveState(neuralrack_plugin_t* plug, void** data, int* size, int isBank) {
    plug->r->saveState(&plug->state);
    *size = strlen(plug->state.c_str());
    //asprintf((char**)data, "%s", plug->state.c_str());
    *data = (void*)plug->state.c_str();
    //fprintf(stderr, "state %s\n", plug->state.c_str());
    //fprintf(stderr, "data %s\n", (char*)*data);
}

void loadState(neuralrack_plugin_t* plug, int size, int isBank) {
    if (plug->state.empty()) return;
    //fprintf(stderr, "Load state %s\n", plug->state.c_str());
    plug->r->readState(plug->state);
}
// --- Dispatcher ---
static intptr_t dispatcher(AEffect* effect, int32_t opCode, int32_t index, intptr_t value, void* ptr, float opt) {
    neuralrack_plugin_t* plug = (neuralrack_plugin_t*)effect->object;
    switch (opCode) {
        case effEditGetRect:
            if (ptr) *(ERect**)ptr = &plug->editorRect;
            break;
        case effGetEffectName:
            strncpy((char*)ptr, "NeuralRack", VestigeMaxNameLen - 1);
            ((char*)ptr)[VestigeMaxNameLen - 1] = '\0';
            return 1;
        case effGetVendorString:
            strncpy((char*)ptr, "brummer", VestigeMaxNameLen - 1);
            ((char*)ptr)[VestigeMaxNameLen - 1] = '\0';
            return 1;
        case effGetProductString:
            strncpy((char*)ptr, "brummer", VestigeMaxNameLen - 1);
            ((char*)ptr)[VestigeMaxNameLen - 1] = '\0';
            return 1;
        case effGetPlugCategory:
            return kPlugCategEffect;
        case effOpen:
            break;
        case effClose:
            plug->r->quitGui();
            delete plug->r;
            free(plug);
            break;
        case effGetParamName:
            getParameterName(effect, index, (char*)ptr);
            break;
        case effSetSampleRate:
            plug->SampleRate = opt;
            plug->r->initEngine((uint32_t)plug->SampleRate, 25, 1);
            loadState(plug, 0, 0);
            break;
        case effEditOpen: {
            Window hostWin = (Window)(size_t)ptr;
            plug->r->startGui();
            plug->r->enableEngine(1);
            plug->r->setParent(hostWin);
            plug->r->showGui();
            break;
        }
        case effEditClose:
            plug->r->quitGui();
            break;
        case effEditIdle:
            break;
        //case effGetProgram:
        case 23: { // effGetChunk
            //fprintf(stderr, "saveState\n");
            void* chunkData = nullptr;
            int   chunkSize = 0;
            saveState(plug, &chunkData, &chunkSize, index); // index=0: program, 1: bank
            *(void**)ptr = chunkData;
            return chunkSize;
        }
        //case effSetProgram:
        case 24: { // effSetChunk
            // index == 0: set plugin state; index == 1: set bank state
            //fprintf(stderr, "loadState\n");
            plug->state = (const char*) ptr;
           // int   chunkSize = value; // value = data size in bytes
           // loadState(plug, chunkSize, index); // index=0: program, 1: bank
            break;
        }
        default: break;
    }
    return 0;
}
