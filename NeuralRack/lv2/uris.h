/*
 * uris.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */

#include <cstdint>
#include <cstdlib>


#include <lv2/core/lv2.h>
#include <lv2/atom/atom.h>
#include <lv2/atom/util.h>
#include <lv2/atom/forge.h>
#include <lv2/midi/midi.h>
#include <lv2/urid/urid.h>
#include <lv2/patch/patch.h>
#include <lv2/options/options.h>
#include <lv2/state/state.h>
#include <lv2/worker/worker.h>
#include <lv2/buf-size/buf-size.h>
#include <lv2/log/log.h>
#include <lv2/log/logger.h>


#define PLUGIN_URI "urn:brummer:neuralrack"
#define XLV2__MODELFILE "urn:brummer:neuralrack#Neural_Model"
#define XLV2__MODELFILE1 "urn:brummer:neuralrack#Neural_Model1"
#define XLV2__IRFILE "urn:brummer:neuralrack#irfile"
#define XLV2__IRFILE1 "urn:brummer:neuralrack#irfile1"
#define XLV2__EQPOS "urn:brummer:neuralrack#eqpos"

#define XLV2__GUI "urn:brummer:neuralrack#gui"


#pragma once

#ifndef URIS_H_
#define URIS_H_

class uris
{
public:

    // LV2 stuff
    LV2_URID_Map*                map;
    LV2_Worker_Schedule*         schedule;
    const LV2_Atom_Sequence*     control;
    LV2_Atom_Sequence*           notify;
    LV2_Atom_Forge               forge;
    LV2_Atom_Forge_Frame         notify_frame;
    LV2_Log_Log*                 log;
    LV2_Log_Logger               logger;

    LV2_URID                     xlv2_model_file;
    LV2_URID                     xlv2_model_file1;
    LV2_URID                     xlv2_ir_file;
    LV2_URID                     xlv2_ir_file1;
    LV2_URID                     xlv2_eq_pos;
    LV2_URID                     xlv2_gui;
    LV2_URID                     atom_Object;
    LV2_URID                     atom_Int;
    LV2_URID                     atom_Float;
    LV2_URID                     atom_Bool;
    LV2_URID                     atom_Vector;
    LV2_URID                     atom_Path;
    LV2_URID                     atom_String;
    LV2_URID                     atom_URID;
    LV2_URID                     atom_eventTransfer;
    LV2_URID                     patch_Put;
    LV2_URID                     patch_Get;
    LV2_URID                     patch_Set;
    LV2_URID                     patch_property;
    LV2_URID                     patch_value;

    inline void map_uris(LV2_URID_Map* map) {
        xlv2_model_file =       map->map(map->handle, XLV2__MODELFILE);
        xlv2_model_file1 =      map->map(map->handle, XLV2__MODELFILE1);
        xlv2_ir_file =          map->map(map->handle, XLV2__IRFILE);
        xlv2_ir_file1 =         map->map(map->handle, XLV2__IRFILE1);
        xlv2_eq_pos =           map->map(map->handle, XLV2__EQPOS);
        xlv2_gui =              map->map(map->handle, XLV2__GUI);
        atom_Object =           map->map(map->handle, LV2_ATOM__Object);
        atom_Int =              map->map(map->handle, LV2_ATOM__Int);
        atom_Float =            map->map(map->handle, LV2_ATOM__Float);
        atom_Bool =             map->map(map->handle, LV2_ATOM__Bool);
        atom_Vector =           map->map(map->handle, LV2_ATOM__Vector);
        atom_Path =             map->map(map->handle, LV2_ATOM__Path);
        atom_String =           map->map(map->handle, LV2_ATOM__String);
        atom_URID =             map->map(map->handle, LV2_ATOM__URID);
        atom_eventTransfer =    map->map(map->handle, LV2_ATOM__eventTransfer);
        patch_Put =             map->map(map->handle, LV2_PATCH__Put);
        patch_Get =             map->map(map->handle, LV2_PATCH__Get);
        patch_Set =             map->map(map->handle, LV2_PATCH__Set);
        patch_property =        map->map(map->handle, LV2_PATCH__property);
        patch_value =           map->map(map->handle, LV2_PATCH__value);
    }

};


#endif
