/*
 *                           0BSD 
 * 
 *                    BSD Zero Clause License
 * 
 *  Copyright (c) 2019 Hermann Meyer
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */


#include <lv2/core/lv2.h>
#include <lv2/ui/ui.h>
#include <lv2/state/state.h>
#include <lv2/worker/worker.h>
#include <lv2/atom/atom.h>
#include <lv2/options/options.h>
#include <lv2/parameters/parameters.h>
#if defined USE_ATOM || defined USE_MIDI
#include <lv2/atom/util.h>
#include <lv2/atom/forge.h>
#include <lv2/midi/midi.h>
#include <lv2/urid/urid.h>
#include <lv2/patch/patch.h>
#endif

#ifndef LV2_UI__scaleFactor
#define LV2_UI__scaleFactor  "http://lv2plug.in/ns/extensions/ui#scaleFactor"
#endif


#define PLUGIN_UI_URI "urn:brummer:neuralrack_ui"

#define XLV2__neural_model "urn:brummer:neuralrack#Neural_Model"
#define XLV2__neural_model1 "urn:brummer:neuralrack#Neural_Model1"
#define XLV2__IRFILE "urn:brummer:neuralrack#irfile"
#define XLV2__IRFILE1 "urn:brummer:neuralrack#irfile1"
#define XLV2__EQPOS "urn:brummer:neuralrack#eqpos"

#define OBJ_BUF_SIZE 1024


typedef struct {
    LV2_URID neural_model;
    LV2_URID neural_model1;
    LV2_URID conv_ir_file;
    LV2_URID conv_ir_file1;
    LV2_URID eq_pos;
    LV2_URID atom_Object;
    LV2_URID atom_Int;
    LV2_URID atom_Float;
    LV2_URID atom_Bool;
    LV2_URID atom_Vector;
    LV2_URID atom_Path;
    LV2_URID atom_String;
    LV2_URID atom_URID;
    LV2_URID atom_eventTransfer;
    LV2_URID patch_Put;
    LV2_URID patch_Get;
    LV2_URID patch_Set;
    LV2_URID patch_property;
    LV2_URID patch_value;
} X11LV2URIs;

#pragma once

#ifndef LV2_PLUGIN_H_
#define LV2_PLUGIN_H_

#ifdef __cplusplus
extern "C" {
#endif


struct Interface {
    LV2_URID_Map* map;
    void *controller;
    LV2UI_Write_Function write_function;
    LV2UI_Resize* resize;
    LV2_Atom_Forge forge;
    X11LV2URIs   uris;    
};

#include "widgets.h"

#ifdef __cplusplus
}
#endif

#endif //LV2_PLUGIN_H_
