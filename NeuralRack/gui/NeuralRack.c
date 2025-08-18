/*
 * NeuralRack.c
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */

#ifdef STANDALONE
#include "standalone.h"
#elif defined(CLAPPLUG)
#include "clapplug.h"
#else
#include "lv2_plugin.cc"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "widgets.cc"

void set_custom_theme(X11_UI *ui) {
    ui->main.color_scheme->normal = (Colors) {
         /* cairo    / r  / g  / b  / a  /  */
        .fg =       { 0.686, 0.729, 0.773, 1.000},
        .bg =       { 0.083, 0.083, 0.083, 1.000},
        .base =     { 0.093, 0.093, 0.093, 1.000},
        .text =     { 0.686, 0.729, 0.773, 1.000},
        .shadow =   { 0.000, 0.000, 0.000, 0.200},
        .frame =    { 0.000, 0.000, 0.000, 1.000},
        .light =    { 0.100, 0.100, 0.100, 1.000}
    };

    ui->main.color_scheme->prelight = (Colors) {
         /* cairo    / r  / g  / b  / a  /  */
        .fg =       { 0.600, 0.600, 0.600, 1.000},
        .bg =       { 0.250, 0.250, 0.250, 1.000},
        .base =     { 0.300, 0.300, 0.300, 1.000},
        .text =     { 1.000, 1.000, 1.000, 1.000},
        .shadow =   { 0.100, 0.100, 0.100, 0.400},
        .frame =    { 0.033, 0.033, 0.033, 1.000},
        .light =    { 0.300, 0.300, 0.300, 1.000}
    };

    ui->main.color_scheme->selected = (Colors) {
         /* cairo    / r  / g  / b  / a  /  */
        .fg =       { 0.900, 0.900, 0.900, 1.000},
        .bg =       { 0.083, 0.083, 0.083, 1.000},
        .base =     { 0.500, 0.180, 0.180, 1.000},
        .text =     { 1.000, 1.000, 1.000, 1.000},
        .shadow =   { 0.800, 0.180, 0.180, 0.200},
        .frame =    { 0.500, 0.180, 0.180, 1.000},
        .light =    { 0.500, 0.180, 0.180, 1.000}
    };

    ui->main.color_scheme->active = (Colors) {
         /* cairo    / r  / g  / b  / a  /  */
        .fg =       { 0.000, 1.000, 1.000, 1.000},
        .bg =       { 0.000, 0.000, 0.000, 1.000},
        .base =     { 0.180, 0.380, 0.380, 1.000},
        .text =     { 0.750, 0.750, 0.750, 1.000},
        .shadow =   { 0.180, 0.380, 0.380, 0.500},
        .frame =    { 0.180, 0.380, 0.380, 1.000},
        .light =    { 0.180, 0.380, 0.380, 1.000}
    };

    ui->main.color_scheme->insensitive = (Colors) {
         /* cairo    / r  / g  / b  / a  /  */
        .fg =       { 0.450, 0.450, 0.450, 0.500},
        .bg =       { 0.100, 0.100, 0.100, 0.500},
        .base =     { 0.000, 0.000, 0.000, 0.500},
        .text =     { 0.900, 0.900, 0.900, 0.500},
        .shadow =   { 0.000, 0.000, 0.000, 0.100},
        .frame =    { 0.000, 0.000, 0.000, 0.500},
        .light =    { 0.100, 0.100, 0.100, 0.500}
    };
}

static void file_load_response(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    ModelPicker* m = (ModelPicker*) w->parent_struct;
    Widget_t *p = (Widget_t*)w->parent;
    X11_UI *ui = (X11_UI*) p->parent_struct;
    if(user_data !=NULL) {
        int old = 0;
        if (ends_with(m->filename, "nam") ||
                ends_with(m->filename, "json") ||
                ends_with(m->filename, "aidax")) {
            old = 1;
        } else if (ends_with(m->filename, "wav") ||
                   ends_with(m->filename, "WAV") ) {
            old = 2;
        }
        free(m->filename);
        m->filename = NULL;
        m->filename = strdup(*(const char**)user_data);

        sendFileName(ui, m, old);

        free(m->filename);
        m->filename = NULL;
        m->filename = strdup("None");
        expose_widget(ui->win);
        ui->loop_counter = 12;
    }
}

void set_ctl_val_from_host(Widget_t *w, float value) {
    xevfunc store = w->func.value_changed_callback;
    w->func.value_changed_callback = dummy_callback;
    adj_set_value(w->adj, value);
    w->func.value_changed_callback = *(*store);
}

static void file_menu_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    ModelPicker* m = (ModelPicker*) w->parent_struct;
    Widget_t *p = (Widget_t*)w->parent;
    X11_UI *ui = (X11_UI*) p->parent_struct;
    X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;
    if (!m->filepicker->file_counter) return;
    int v = (int)adj_get_value(w->adj);
    if (v >= m->filepicker->file_counter) {
        free(ps->fname);
        ps->fname = NULL;
        asprintf(&ps->fname, "%s", "None");
    } else {
        free(ps->fname);
        ps->fname = NULL;
        asprintf(&ps->fname, "%s%s%s", m->dir_name, PATH_SEPARATOR, m->filepicker->file_names[v]);
    }
    file_load_response(m->filebutton, (void*)&ps->fname);
}

void plugin_set_window_size(int *w,int *h,const char * plugin_uri) {
    (*w) = 620; //set initial width of main window
    (*h) = 580; //set initial height of main window
}

const char* plugin_set_name() {
    return "Neural Rack"; //set plugin name to display on UI
}

void plugin_create_controller_widgets(X11_UI *ui, const char * plugin_uri) {

    ui->win->label = plugin_set_name();
    // connect the expose func
    ui->win->func.expose_callback = draw_window;

    X11_UI_Private_t *ps =(X11_UI_Private_t*)malloc(sizeof(X11_UI_Private_t));
    ui->private_ptr = (void*)ps;
    ps->ma.filename = strdup("None");
    ps->mb.filename = strdup("None");
    ps->ir.filename = strdup("None");
    ps->ir1.filename = strdup("None");
    ps->ma.dir_name = NULL;
    ps->mb.dir_name = NULL;
    ps->ir.dir_name = NULL;
    ps->ir1.dir_name = NULL;
    ps->fname = NULL;
    ps->ma.filepicker = (FilePicker*)malloc(sizeof(FilePicker));
    fp_init(ps->ma.filepicker, "/");
    asprintf(&ps->ma.filepicker->filter ,"%s", ".nam|.aidax|.json");
    ps->ma.filepicker->use_filter = 1;
    ps->mb.filepicker = (FilePicker*)malloc(sizeof(FilePicker));
    fp_init(ps->mb.filepicker, "/");
    asprintf(&ps->mb.filepicker->filter ,"%s", ".nam|.aidax|.json");
    ps->mb.filepicker->use_filter = 1;
    ps->ir.filepicker = (FilePicker*)malloc(sizeof(FilePicker));
    fp_init(ps->ir.filepicker, "/");
    asprintf(&ps->ir.filepicker->filter ,"%s", ".wav|.WAV");
    ps->ir.filepicker->use_filter = 1;
    ps->ir1.filepicker = (FilePicker*)malloc(sizeof(FilePicker));
    fp_init(ps->ir1.filepicker, "/");
    asprintf(&ps->ir1.filepicker->filter ,"%s", ".wav|.WAV");
    ps->ir1.filepicker->use_filter = 1;

    ui->widget[15] = add_lv2_slider (ui->widget[15], ui->win, 20, "Buffer", ui, 50,  12, 40, 40);
    set_adjustment(ui->widget[15]->adj, 0.0, 0.0, 0.0, 2.0, 1.0, CL_CONTINUOS);

    ui->widget[16] = add_lv2_label (ui->widget[16], ui->win, 22, "Latency", ui, 90,  22, 130, 30);
    ui->widget[17] = add_lv2_label (ui->widget[17], ui->win, 23, "Xrun", ui, 410,  22, 100, 30);

    ui->widget[10] = add_lv2_switch (ui->widget[10], ui->win, 14, "Enable", ui, 505,  12, 50, 50);

// noisegate
    ui->elem[4] = create_widget(&ui->main, ui->win, 10, 60, 600, 70);
    ui->elem[4]->parent_struct = ui;
    ui->elem[4]->label = "Noise Gate";
    ui->elem[4]->data = 1;

    // rack mount background colour
    set_widget_color(ui->elem[4], (Color_state)0, (Color_mod)1, 0.529, 0.529, 0.529, 1.0);
    // rack mount foreground colour 
    set_widget_color(ui->elem[4], (Color_state)0, (Color_mod)0, 0.078, 0.078, 0.078, 0.5);
    ui->elem[4]->func.expose_callback = draw_eq_window;

    ui->widget[26] = add_eq_button (ui->widget[26], ui->elem[4], 32, "", ui, 25,  20, 30, 30);
    set_widget_color(ui->widget[26], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    widget_get_png(ui->widget[26], LDVAR(exit__png));

    ui->widget[25] = add_lv2_knob (ui->widget[25], ui->elem[4], 31, "", ui, 510, 10, 60, 60); //Threshold
    set_adjustment(ui->widget[25]->adj, 0.017, 0.017, 0.01, 0.31, 0.001, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[25], (Color_state)0, (Color_mod)0, 0.235, 0.215, 0.282, 1.0);
    // controller background colour
    set_widget_color(ui->widget[25], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[25], (Color_state)1, (Color_mod)0, 0.335, 0.315, 0.382, 1.0);

// slot A Pedal Profile
    ui->elem[0] = create_widget(&ui->main, ui->win, 10, 130, 600, 110);
    ui->elem[0]->parent_struct = ui;
    ui->elem[0]->label = "Pedal Profile";
    ui->elem[0]->data = 1;
    // rack mount background colour
    set_widget_color(ui->elem[0], (Color_state)0, (Color_mod)1, 0.306, 0.510, 0.584, 1.0);
    // rack mount foreground colour 
    set_widget_color(ui->elem[0], (Color_state)0, (Color_mod)0, 0.078, 0.078, 0.078, 0.5);
    ui->elem[0]->func.expose_callback = draw_elem;

    ui->widget[0] = add_lv2_knob (ui->widget[0], ui->elem[0], 2, "Input", ui, 430, 15, 70, 80);
    set_adjustment(ui->widget[0]->adj, 0.0, 0.0, -20.0, 20.0, 0.2, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[0], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[0], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[0], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);

    ui->widget[1] = add_lv2_knob (ui->widget[1], ui->elem[0], 3, "Output ", ui, 510, 15, 70, 80);
    set_adjustment(ui->widget[1]->adj, 0.0, 0.0, -20.0, 20.0, 0.2, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[1], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[1], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[1], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);


    ps->ma.fbutton = add_lv2_button(ps->ma.fbutton, ui->elem[0], "", ui, 365, 44, 22, 30);
    ps->ma.fbutton->parent_struct = (void*)&ps->ma;
    combobox_set_pop_position(ps->ma.fbutton, 0);
    combobox_set_entry_length(ps->ma.fbutton, 50);
    combobox_add_entry(ps->ma.fbutton, "None");
    ps->ma.fbutton->func.value_changed_callback = file_menu_callback;

    ps->ma.filebutton = add_lv2_file_button (ps->ma.filebutton, ui->elem[0], -1, "Neural Model", ui, 30, 48, 25, 25);
    ps->ma.filebutton->parent_struct = (void*)&ps->ma;
    ps->ma.filebutton->func.user_callback = file_load_response;

    ui->widget[8] = add_lv2_toggle_button (ui->widget[8], ui->elem[0], 12, "", ui, 60, 48, 25, 25);
    ui->widget[11] = add_lv2_erase_button (ui->widget[11], ui->elem[0], 15, "", ui, 390, 48, 25, 25);

// EQ
    ui->elem[3] = create_widget(&ui->main, ui->win, 10, 240, 600, 110);
    ui->elem[3]->parent_struct = ui;
    ui->elem[3]->label = "6 Band EQ";
    // rack mount background colour
    set_widget_color(ui->elem[3], (Color_state)0, (Color_mod)1, 0.569, 0.271, 0.310,1.0);
    // rack mount foreground colour 
    set_widget_color(ui->elem[3], (Color_state)0, (Color_mod)0, 0.078, 0.078, 0.078,0.5);
    ui->elem[3]->func.expose_callback = draw_eq_window;

    ui->widget[24] = add_eq_button (ui->widget[24], ui->elem[3], 30, "", ui, 25,  20, 30, 30);
    set_widget_color(ui->widget[24], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    widget_get_png(ui->widget[24], LDVAR(exit__png));

    ui->widget[18] = add_lv2_knob (ui->widget[18], ui->elem[3], 24, "30 Hz", ui, 220, 12, 60, 80);
    set_adjustment(ui->widget[18]->adj, -20.0, -20.0, -20.0, 20.0, 0.1, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[18], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[18], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[18], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);

    ui->widget[19] = add_lv2_knob (ui->widget[19], ui->elem[3], 25, "125 Hz", ui, 280, 12, 60, 80);
    set_adjustment(ui->widget[19]->adj, 0.0, 0.0, -20.0, 20.0, 0.1, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[19], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[19], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[19], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);

    ui->widget[20] = add_lv2_knob (ui->widget[20], ui->elem[3], 26, "500 Hz", ui, 340, 12, 60, 80);
    set_adjustment(ui->widget[20]->adj, 0.0, 0.0, -20.0, 20.0, 0.1, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[20], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[20], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[20], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);

    ui->widget[21] = add_lv2_knob (ui->widget[21], ui->elem[3], 27, "2 kHz", ui, 400, 12, 60, 80);
    set_adjustment(ui->widget[21]->adj, 0.0, 0.0, -20.0, 20.0, 0.1, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[21], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[21], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[21], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);

    ui->widget[22] = add_lv2_knob (ui->widget[22], ui->elem[3], 28, "8 kHz", ui, 460, 12, 60, 80);
    set_adjustment(ui->widget[22]->adj, 0.0, 0.0, -20.0, 20.0, 0.1, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[22], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[22], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[22], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);

    ui->widget[23] = add_lv2_knob (ui->widget[23], ui->elem[3], 29, "16 kHz", ui, 520, 12, 60, 80);
    set_adjustment(ui->widget[23]->adj, -20.0, -20.0, -20.0, 20.0, 0.1, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[23], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[23], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[23], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);

// sloat B Amp Profile
    ui->elem[1] = create_widget(&ui->main, ui->win, 10, 350, 600, 110);
    ui->elem[1]->parent_struct = ui;
    ui->elem[1]->label = "Amp Profile";
    ui->elem[1]->data = 2;
    // rack mount background colour
    set_widget_color(ui->elem[1], (Color_state)0, (Color_mod)1, 0.725, 0.592, 0.388, 1.0);
    // rack mount foreground colour 
    set_widget_color(ui->elem[1], (Color_state)0, (Color_mod)0, 0.078, 0.078, 0.078, 0.5);
    ui->elem[1]->func.expose_callback = draw_elem;

    ui->widget[7] = add_lv2_knob (ui->widget[7], ui->elem[1], 11, "Input", ui, 430, 15, 70, 80);
    set_adjustment(ui->widget[7]->adj, 0.0, 0.0, -20.0, 20.0, 0.2, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[7], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[7], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[7], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);

    ui->widget[2] = add_lv2_knob (ui->widget[2], ui->elem[1], 4, "Output", ui, 510, 15, 70, 80);
   // widget_get_png(ui->widget[2], LDVAR(knob1_png));
    set_adjustment(ui->widget[2]->adj, 0.0, 0.0, -20.0, 20.0, 0.2, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[2], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631, 1.0);
    // controller background colour
    set_widget_color(ui->widget[2], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[2], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737, 1.0);

    ps->mb.fbutton = add_lv2_button(ps->mb.fbutton, ui->elem[1], "", ui, 365, 44, 22, 30);
    ps->mb.fbutton->parent_struct = (void*)&ps->mb;
    combobox_set_pop_position(ps->mb.fbutton, 0);
    combobox_set_entry_length(ps->mb.fbutton, 60);
    combobox_add_entry(ps->mb.fbutton, "None");
    ps->mb.fbutton->func.value_changed_callback = file_menu_callback;

    ps->mb.filebutton = add_lv2_file_button (ps->mb.filebutton, ui->elem[1], -2, "Neural Model", ui, 30, 48, 25, 25);
    ps->mb.filebutton->parent_struct = (void*)&ps->mb;
    ps->mb.filebutton->func.user_callback = file_load_response;

    ui->widget[9] = add_lv2_toggle_button (ui->widget[9], ui->elem[1], 13, "", ui, 60, 48, 25, 25);
    ui->widget[12] = add_lv2_erase_button (ui->widget[12], ui->elem[1], 16, "", ui, 390, 48, 25, 25);

// IR
    ui->elem[2] = create_widget(&ui->main, ui->win, 10, 460, 600, 110);
    ui->elem[2]->parent_struct = ui;
    ui->elem[2]->label = "IR Loader";
    // rack mount background colour
    set_widget_color(ui->elem[2], (Color_state)0, (Color_mod)1, 0.176, 0.176, 0.176,1.0);
    // rack mount foreground colour 
    set_widget_color(ui->elem[2], (Color_state)0, (Color_mod)0, 0.322, 0.322, 0.322,1.0);
    ui->elem[2]->func.expose_callback = draw_ir_elem;

    ui->widget[3] = add_lv2_knob (ui->widget[3], ui->elem[2], 7, "Gain (L)", ui, 55, 15, 70, 80);
    set_adjustment(ui->widget[3]->adj, 0.0, 0.0, -20.0, 20.0, 0.2, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[3], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631,1.0);
    // controller background colour
    set_widget_color(ui->widget[3], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[3], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737,1.0);

    ui->widget[28] = add_lv2_knob (ui->widget[28], ui->elem[2], 34, "Mix", ui, 55, 15, 70, 80);
    set_adjustment(ui->widget[28]->adj, 0.5, 0.5, 0.0, 1.0, 0.01, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[28], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631,1.0);
    // controller background colour
    set_widget_color(ui->widget[28], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[28], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737,1.0);

    ps->ir.fbutton = add_lv2_button(ps->ir.fbutton, ui->elem[2], "", ui, 445, 20, 22, 30);
    ps->ir.fbutton->parent_struct = (void*)&ps->ir;
    combobox_set_pop_position(ps->ir.fbutton, 0);
    combobox_set_entry_length(ps->ir.fbutton, 60);
    combobox_add_entry(ps->ir.fbutton, "None");
    ps->ir.fbutton->func.value_changed_callback = file_menu_callback;

    ps->ir.filebutton = add_lv2_irfile_button (ps->ir.filebutton, ui->elem[2], -3, "IR File", ui, 140, 24, 25, 25);
    ps->ir.filebutton->parent_struct = (void*)&ps->ir;
    ps->ir.filebutton->func.user_callback = file_load_response;

    ui->widget[5] = add_lv2_toggle_button (ui->widget[5], ui->elem[2], 9, "", ui, 170, 24, 25, 25);
    ui->widget[13] = add_lv2_erase_button (ui->widget[13], ui->elem[2], 17, "", ui, 470, 24, 25, 25);

//IR 1
    ui->widget[4] = add_lv2_knob (ui->widget[4], ui->elem[2], 8, "Gain (R)", ui, 510, 15, 70, 80);
    set_adjustment(ui->widget[4]->adj, 0.0, 0.0, -20.0, 20.0, 0.2, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[4], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631,1.0);
    // controller background colour
    set_widget_color(ui->widget[4], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[4], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737,1.0);

    ui->widget[29] = add_lv2_knob (ui->widget[29], ui->elem[2], 35, "Master", ui, 510, 15, 70, 80);
    set_adjustment(ui->widget[29]->adj, 0.0, 0.0, -20.0, 20.0, 0.2, CL_CONTINUOS);
    // controller label colour
    set_widget_color(ui->widget[29], (Color_state)0, (Color_mod)0, 0.592, 0.612, 0.631,1.0);
    // controller background colour
    set_widget_color(ui->widget[29], (Color_state)0, (Color_mod)1, 0.083, 0.083, 0.083, 1.0);
    // controller label colour hover
    set_widget_color(ui->widget[29], (Color_state)1, (Color_mod)0, 0.694, 0.714, 0.737,1.0);

    ps->ir1.fbutton = add_lv2_button(ps->ir1.fbutton, ui->elem[2], "", ui, 445, 64, 22, 30);
    ps->ir1.fbutton->parent_struct = (void*)&ps->ir1;
    combobox_set_pop_position(ps->ir1.fbutton, 0);
    combobox_set_entry_length(ps->ir1.fbutton, 60);
    combobox_add_entry(ps->ir1.fbutton, "None");
    ps->ir1.fbutton->func.value_changed_callback = file_menu_callback;

    ps->ir1.filebutton = add_lv2_irfile_button (ps->ir1.filebutton, ui->elem[2], -4, "IR File", ui, 140, 68, 25, 25);
    ps->ir1.filebutton->parent_struct = (void*)&ps->ir1;
    ps->ir1.filebutton->func.user_callback = file_load_response;

    ui->widget[6] = add_lv2_toggle_button (ui->widget[6], ui->elem[2], 10, "", ui, 170, 68, 25, 25);
    ui->widget[14] = add_lv2_erase_button (ui->widget[14], ui->elem[2], 18, "", ui, 470, 68, 25, 25);

    // switch between Stereo and Mix output
    ui->widget[27] = add_lv2_vswitch (ui->widget[27], ui->elem[2], 33, "Stereo", ui, 20, 18, 35, 80);

}


void plugin_cleanup(X11_UI *ui) {
    X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;
    free(ps->fname);
    free(ps->ma.filename);
    free(ps->mb.filename);
    free(ps->ma.dir_name);
    free(ps->mb.dir_name);
    free(ps->ir.filename);
    free(ps->ir.dir_name);
    free(ps->ir1.filename);
    free(ps->ir1.dir_name);
    fp_free(ps->ma.filepicker);
    free(ps->ma.filepicker);
    fp_free(ps->mb.filepicker);
    free(ps->mb.filepicker);
    fp_free(ps->ir.filepicker);
    free(ps->ir.filepicker);
    fp_free(ps->ir1.filepicker);
    free(ps->ir1.filepicker);
    // clean up used sources when needed
}


#ifdef __cplusplus
}
#endif
