/*
 * NeuralRackParameter.cc
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */

#include "NeuralRackParameter.h"

#include <string>
#include <cstdio>
#include <atomic>

const std::vector<NeuralRackParameter> neuralrack_parameters = {
    // id,                 name,            module,   min,max,def,step,   flags

    {PARAM_INPUT_GAIN,    "Input Gain A",   "Pedal", -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_INPUT_GAIN1,   "Input Gain B",   "Amp",   -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_OUTPUT_GAIN,   "Output Gain A",  "Pedal", -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_OUTPUT_GAIN1,  "Output Gain B",  "Amp",   -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_IROUTPUT_GAIN, "IR Out Gain L",  "IR",    -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_IROUTPUT_GAIN1,"IR Out Gain R",  "IR1",   -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},

    {PARAM_THRESHOLD,     "Gate Thresh",    "NoiseGate", 0.01, 0.31, 0.017, 0.001, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_EQ_ONOFF,      "EQ Enable",      "EQ",     0,1,0,1, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED},
    {PARAM_NG_ONOFF,      "Gate Enable",    "NoiseGate",0,1,0,1, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED},
    {PARAM_NORMSLOT_A,    "Norm Slot A",    "Pedal",  0,1,0,1, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED},
    {PARAM_NORMSLOT_B,    "Norm Slot B",    "Amp",    0,1,0,1, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED},
    {PARAM_BYPASS,        "Enable",         "Global", 0,1,0,1, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED},
    {PARAM_BUFFERED,      "Buffered Mode",  "Global", 0,2,0,1, CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_IS_STEPPED},

    {PARAM_EQ_BAND0,      "EQ Band 1",      "EQ",    -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_EQ_BAND1,      "EQ Band 2",      "EQ",    -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_EQ_BAND2,      "EQ Band 3",      "EQ",    -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_EQ_BAND3,      "EQ Band 4",      "EQ",    -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_EQ_BAND4,      "EQ Band 5",      "EQ",    -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
    {PARAM_EQ_BAND5,      "EQ Band 6",      "EQ",    -20,20,0,0.1, CLAP_PARAM_IS_AUTOMATABLE},
};

void NeuralRackParams::reset(neuralrack_plugin_t *plug) {
    for (uint32_t i = 0; i < PARAM_COUNT; i++) {
        const auto& def = neuralrack_parameters[i];
        setParam(plug, i, def.def);
    }
    // inform the GUI thread that parameters was changed by the host
    plug->r->paramChanged.store(true, std::memory_order_release);
}

double NeuralRackParams::getParam(neuralrack_plugin_t *plug, int idx) const {
    neuralrack::Engine *engine = plug->r->getEngine();
    switch(idx) {
        case PARAM_INPUT_GAIN: return engine->inputGain;
        case PARAM_INPUT_GAIN1: return engine->inputGain1;
        case PARAM_OUTPUT_GAIN: return engine->outputGain;
        case PARAM_OUTPUT_GAIN1: return engine->outputGain1;
        case PARAM_IROUTPUT_GAIN: return engine->IRoutputGain;
        case PARAM_IROUTPUT_GAIN1: return engine->IRoutputGain1;
        case PARAM_THRESHOLD: return engine->ngate->threshold;
        case PARAM_EQ_ONOFF: return engine->eqOnOff;
        case PARAM_NG_ONOFF: return engine->ngOnOff;
        case PARAM_NORMSLOT_A: return engine->normSlotA;
        case PARAM_NORMSLOT_B: return engine->normSlotB;
        case PARAM_BYPASS: return engine->bypass;
        case PARAM_BUFFERED: return engine->buffered;
        case PARAM_EQ_BAND0: return engine->peq->fVslider1;
        case PARAM_EQ_BAND1: return engine->peq->fVslider0;
        case PARAM_EQ_BAND2: return engine->peq->fVslider2;
        case PARAM_EQ_BAND3: return engine->peq->fVslider3;
        case PARAM_EQ_BAND4: return engine->peq->fVslider4;
        case PARAM_EQ_BAND5: return engine->peq->fVslider5;
        default: return 0.0;
    }
}
void NeuralRackParams::setParam(neuralrack_plugin_t *plug, int idx, double value) {
    neuralrack::Engine *engine = plug->r->getEngine();
    switch(idx) {
        case PARAM_INPUT_GAIN: engine->inputGain = value; break;
        case PARAM_INPUT_GAIN1: engine->inputGain1 = value; break;
        case PARAM_OUTPUT_GAIN: engine->outputGain = value; break;
        case PARAM_OUTPUT_GAIN1: engine->outputGain1 = value; break;
        case PARAM_IROUTPUT_GAIN: engine->IRoutputGain = value; break;
        case PARAM_IROUTPUT_GAIN1: engine->IRoutputGain1 = value; break;
        case PARAM_THRESHOLD: engine->ngate->threshold = value; break;
        case PARAM_EQ_ONOFF: engine->eqOnOff = value; break;
        case PARAM_NG_ONOFF: engine->ngOnOff = value; break;
        case PARAM_NORMSLOT_A: engine->normSlotA = value; break;
        case PARAM_NORMSLOT_B: engine->normSlotB = value; break;
        case PARAM_BYPASS: engine->bypass = value; break;
        case PARAM_BUFFERED: engine->buffered = value; break;
        case PARAM_EQ_BAND0: engine->peq->fVslider1 = value; break;
        case PARAM_EQ_BAND1: engine->peq->fVslider0 = value; break;
        case PARAM_EQ_BAND2: engine->peq->fVslider2 = value; break;
        case PARAM_EQ_BAND3: engine->peq->fVslider3 = value; break;
        case PARAM_EQ_BAND4: engine->peq->fVslider4 = value; break;
        case PARAM_EQ_BAND5: engine->peq->fVslider5 = value; break;
        default: break;
    }
    // inform the GUI thread that a parameter was changed by the host
    plug->r->paramChanged.store(true, std::memory_order_release);
}
