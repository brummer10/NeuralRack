
/*
 * widgets.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */

// xwidgets.h includes xputty.h and all defined widgets from Xputty
#include "xwidgets.h"
#include "xfile-dialog.h"

#pragma once

#ifndef WIDGETS_H_
#define WIDGETS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CONTROLS 27

#define GUI_ELEMENTS 5

#define TAB_ELEMENTS 0

typedef struct Interface Interface;

typedef struct {
    Widget_t *fbutton;
    Widget_t *filebutton;
    FilePicker *filepicker;
    char *filename;
    char *dir_name;
} ModelPicker;

typedef struct {
    ModelPicker ma;
    ModelPicker mb;
    ModelPicker ir;
    ModelPicker ir1;
    char *fname;
} X11_UI_Private_t;

// main window struct
typedef struct {
    void *parentXwindow;
    Xputty main;
    Widget_t *win;
    Widget_t *widget[CONTROLS];
    Widget_t *elem[GUI_ELEMENTS];
    Widget_t *tab_elem[TAB_ELEMENTS];
    unsigned int f_index;
    void *private_ptr;
    int need_resize;
    int loop_counter;
    bool uiKnowSampleRate;
    int uiSampleRate;
    bool setVerbose;
    Interface itf;
} X11_UI;

// set the plugin initial window size
void plugin_set_window_size(int *w,int *h,const char * plugin_uri);

// set custom theme 
void set_custom_theme(X11_UI *ui);

// create all needed controller 
void plugin_create_controller_widgets(X11_UI *ui, const char * plugin_uri);

// send value changes to the host
void sendValueChanged(X11_UI *ui, int port, float value);

// send a file name to the host
void sendFileName(X11_UI *ui, ModelPicker* m, int old);

int ends_with(const char* name, const char* extension);

// free used mem on exit
void plugin_cleanup(X11_UI *ui);

// set a callback to NULL
static void dummy_callback(void *w_, void* user_data) {}

#ifdef __cplusplus
}
#endif

#endif
