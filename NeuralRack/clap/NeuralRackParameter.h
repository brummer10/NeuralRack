/*
 * NeuralRackParameter.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */


#include <string>
#include <vector>
#include <cstring>

#pragma once
#ifndef NEURALRACKPARAMETER_H_
#define NEURALRACKPARAMETER_H_

// Enum for parameter indices
enum NeuralRackParamID {
    PARAM_INPUT_GAIN = 0,
    PARAM_INPUT_GAIN1,
    PARAM_OUTPUT_GAIN,
    PARAM_OUTPUT_GAIN1,
    PARAM_IROUTPUT_GAIN,
    PARAM_IROUTPUT_GAIN1,
    PARAM_THRESHOLD,
    PARAM_EQ_ONOFF,
    PARAM_NG_ONOFF,
    PARAM_NORMSLOT_A,
    PARAM_NORMSLOT_B,
    PARAM_BYPASS,
    PARAM_BUFFERED,
    PARAM_EQ_BAND0,
    PARAM_EQ_BAND1,
    PARAM_EQ_BAND2,
    PARAM_EQ_BAND3,
    PARAM_EQ_BAND4,
    PARAM_EQ_BAND5,
    PARAM_COUNT // must be last
};

struct NeuralRackParameter {
    int id; // clap_id
    std::string name;
    std::string module;
    double min, max, def, step;
    uint32_t flags;
};

class NeuralRackParams {
public:
    void reset(neuralrack_plugin_t *plug);
    double getParam(neuralrack_plugin_t *plug, int idx) const;
    void setParam(neuralrack_plugin_t *plug, int idx, double value);

};


//extern const std::vector<NeuralRackParameter> neuralrack_parameters;
#endif
