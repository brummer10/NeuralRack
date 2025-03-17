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


#include "lv2_plugin.h"

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                LV2 function calls
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

static inline void map_x11ui_uris(LV2_URID_Map* map, X11LV2URIs* uris) {
    uris->neural_model = map->map(map->handle, XLV2__neural_model);
    uris->neural_model1 = map->map(map->handle, XLV2__neural_model1);
    uris->conv_ir_file = map->map(map->handle, XLV2__IRFILE);
    uris->conv_ir_file1 = map->map(map->handle, XLV2__IRFILE1);
    uris->atom_Object = map->map(map->handle, LV2_ATOM__Object);
    uris->atom_Int = map->map(map->handle, LV2_ATOM__Int);
    uris->atom_Float = map->map(map->handle, LV2_ATOM__Float);
    uris->atom_Bool = map->map(map->handle, LV2_ATOM__Bool);
    uris->atom_Vector = map->map(map->handle, LV2_ATOM__Vector);
    uris->atom_Path = map->map(map->handle, LV2_ATOM__Path);
    uris->atom_String = map->map(map->handle, LV2_ATOM__String);
    uris->atom_URID = map->map(map->handle, LV2_ATOM__URID);
    uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
    uris->patch_Put = map->map(map->handle, LV2_PATCH__Put);
    uris->patch_Get = map->map(map->handle, LV2_PATCH__Get);
    uris->patch_Set = map->map(map->handle, LV2_PATCH__Set);
    uris->patch_property = map->map(map->handle, LV2_PATCH__property);
    uris->patch_value = map->map(map->handle, LV2_PATCH__value);
}

static inline LV2_Atom* write_set_file(const LV2_URID urid, LV2_Atom_Forge* forge,
                const X11LV2URIs* uris, const char* filename) {
    LV2_Atom_Forge_Frame frame;
    LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_object(
                        forge, &frame, 1, uris->patch_Set);
    lv2_atom_forge_key(forge, uris->patch_property);
    lv2_atom_forge_urid(forge, urid);
    lv2_atom_forge_key(forge, uris->patch_value);
    lv2_atom_forge_path(forge, filename, strlen(filename) + 1);
    lv2_atom_forge_pop(forge, &frame);
    return set;
}

void sendValueChanged(X11_UI *ui, int port, float value) {
    ui->itf.write_function(ui->itf.controller, port,sizeof(float),0,&value);
}

int ends_with(const char* name, const char* extension) {
    const char* ldot = strrchr(name, '.');
    if (ldot != NULL) {
        size_t length = strlen(extension);
        return strncmp(ldot + 1, extension, length) == 0;
    }
    return 0;
}

void sendFileName(X11_UI *ui, ModelPicker* m, int old) {
    X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;
    LV2_URID urid;
    if ((strcmp(m->filename, "None") == 0)) {
        if (old == 1) {
            if ( m == &ps->ma) urid = ui->itf.uris.neural_model;
            else urid = ui->itf.uris.neural_model1;
        } else if (old == 2) {
            if ( m == &ps->ir) urid = ui->itf.uris.conv_ir_file;
            else urid = ui->itf.uris.conv_ir_file1;
        } else return;
    } else if (ends_with(m->filename, "nam") ||
               ends_with(m->filename, "json") ||
               ends_with(m->filename, "aidax")) {
        if ( m == &ps->ma) urid = ui->itf.uris.neural_model;
        else  urid = ui->itf.uris.neural_model1;
    } else if (ends_with(m->filename, "wav")) {
        if ( m == &ps->ir) urid = ui->itf.uris.conv_ir_file;
        else urid = ui->itf.uris.conv_ir_file1;
    } else return;
    uint8_t obj_buf[OBJ_BUF_SIZE];
    lv2_atom_forge_set_buffer(&ui->itf.forge, obj_buf, OBJ_BUF_SIZE);
    LV2_Atom* msg = (LV2_Atom*)write_set_file(urid, &ui->itf.forge, &ui->itf.uris, m->filename);
    ui->itf.write_function(ui->itf.controller, 5, lv2_atom_total_size(msg),
                       ui->itf.uris.atom_eventTransfer, msg);
}

Widget_t *get_widget_from_urid(X11_UI *ui, const LV2_URID urid) {
    X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;
    if (urid == ui->itf.uris.neural_model)
        return ps->ma.filebutton;
    else if (urid == ui->itf.uris.neural_model1)
        return ps->mb.filebutton;
    else if (urid == ui->itf.uris.conv_ir_file)
        return ps->ir.filebutton;
    else if (urid == ui->itf.uris.conv_ir_file1)
        return ps->ir1.filebutton;
    return NULL;
}

static inline const LV2_Atom* read_set_file(const X11LV2URIs* uris, X11_UI *ui,
                                    ModelPicker **m, const LV2_Atom_Object* obj) {
    if (obj->body.otype != uris->patch_Set) {
        return NULL;
    }
    const LV2_Atom* property = NULL;
    lv2_atom_object_get(obj, uris->patch_property, &property, 0);
    if (property == NULL) return NULL;
    Widget_t *w = get_widget_from_urid(ui,((LV2_Atom_URID*)property)->body);
    if (!w || (property->type != uris->atom_URID)) {
        return NULL;
    }
    const LV2_Atom* file_path = NULL;
    lv2_atom_object_get(obj, uris->patch_value, &file_path, 0);
    if (!file_path || (file_path->type != uris->atom_Path)) {
        return NULL;
    }
    *(m) = (ModelPicker*) w->parent_struct;
    return file_path;
}

static void rebuild_file_menu(ModelPicker *m) {
    xevfunc store = m->fbutton->func.value_changed_callback;
    m->fbutton->func.value_changed_callback = dummy_callback;
    combobox_delete_entrys(m->fbutton);
    fp_get_files(m->filepicker, m->dir_name, 0, 1);
    int active_entry = m->filepicker->file_counter-1;
    unsigned int i = 0;
    for(;i<m->filepicker->file_counter;i++) {
        combobox_add_entry(m->fbutton, m->filepicker->file_names[i]);
        if (strcmp(basename(m->filename),m->filepicker->file_names[i]) == 0) 
            active_entry = i;
    }
    combobox_add_entry(m->fbutton, "None");
    adj_set_value(m->fbutton->adj, active_entry);
    combobox_set_menu_size(m->fbutton, min(14, m->filepicker->file_counter+1));
    m->fbutton->func.value_changed_callback = store;
}

static inline void get_file(const LV2_Atom* file_uri, X11_UI* ui, ModelPicker *m) {
    const char* uri = (const char*)LV2_ATOM_BODY(file_uri);
    if (strlen(uri) && (strcmp(uri, "None") != 0)) {
        if (strcmp(uri, (const char*)m->filename) !=0) {
            free(m->filename);
            m->filename = NULL;
            m->filename = strdup(uri);
            char *dn = strdup(dirname((char*)uri));
            if (m->dir_name == NULL || strcmp((const char*)m->dir_name,
                                                    (const char*)dn) !=0) {
                free(m->dir_name);
                m->dir_name = NULL;
                m->dir_name = strdup(dn);
                FileButton *filebutton = (FileButton*)m->filebutton->private_struct;
                filebutton->path = m->dir_name;
                rebuild_file_menu(m);
            }
            free(dn);
            expose_widget(ui->win);
        }
    } else if (strcmp(m->filename, "None") != 0) {
        free(m->filename);
        m->filename = NULL;
        m->filename = strdup("None");
        expose_widget(ui->win);
    }
}

void plugin_port_event(LV2UI_Handle handle, uint32_t port_index,
                        uint32_t buffer_size, uint32_t format,
                        const void * buffer) {
    X11_UI* ui = (X11_UI*)handle;
    const X11LV2URIs* uris = &ui->itf.uris;
    if (format == ui->itf.uris.atom_eventTransfer) {
        const LV2_Atom* atom = (LV2_Atom*)buffer;
        if (atom->type == ui->itf.uris.atom_Object) {
            const LV2_Atom_Object* obj = (LV2_Atom_Object*)atom;
            if (obj->body.otype == uris->patch_Set) {
                ModelPicker *m = NULL;
                const LV2_Atom* file_uri = read_set_file(uris, ui, &m, obj);
                if (file_uri && m) {
                    get_file(file_uri, ui, m);
                }
            }
        }
    }
    // port value change message from host
    // do special stuff when needed
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                the main LV2 handle->XWindow
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

// init the xwindow and return the LV2UI handle
static LV2UI_Handle instantiate(const LV2UI_Descriptor * descriptor,
            const char * plugin_uri, const char * bundle_path,
            LV2UI_Write_Function write_function,
            LV2UI_Controller controller, LV2UI_Widget * widget,
            const LV2_Feature * const * features) {

    X11_UI* ui = (X11_UI*)malloc(sizeof(X11_UI));

    if (!ui) {
        fprintf(stderr,"ERROR: failed to instantiate plugin with URI %s\n", plugin_uri);
        return NULL;
    }

    LV2_Options_Option *opts = NULL;

    ui->parentXwindow = 0;
    ui->private_ptr = NULL;
    ui->need_resize = 1;
    ui->loop_counter = 4;
    ui->uiKnowSampleRate = false;
    ui->uiSampleRate = 0;
    ui->f_index = 0;

    int i = 0;
    for(;i<CONTROLS;i++)
        ui->widget[i] = NULL;
    i = 0;
    for(;i<GUI_ELEMENTS;i++)
        ui->elem[i] = NULL;

    i = 0;
    for (; features[i]; ++i) {
        if (!strcmp(features[i]->URI, LV2_UI__parent)) {
            ui->parentXwindow = features[i]->data;
        } else if(!strcmp(features[i]->URI, LV2_OPTIONS__options)) {
            opts = (LV2_Options_Option*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_UI__resize)) {
            ui->itf.resize = (LV2UI_Resize*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_URID_URI "#map")) {
            ui->itf.map = (LV2_URID_Map*)features[i]->data;
        }
    }

    if (ui->parentXwindow == NULL)  {
        fprintf(stderr, "ERROR: Failed to open parentXwindow for %s\n", plugin_uri);
        free(ui);
        return NULL;
    }

    map_x11ui_uris(ui->itf.map, &ui->itf.uris);
    lv2_atom_forge_init(&ui->itf.forge, ui->itf.map);
    // init Xputty
    main_init(&ui->main);
    set_custom_theme(ui);

    float scale = 1.0;
    if (opts != NULL) {
        const LV2_URID ui_scaleFactor = ui->itf.map->map(ui->itf.map->handle, LV2_UI__scaleFactor);
        const LV2_URID atom_Float = ui->itf.map->map(ui->itf.map->handle, LV2_ATOM__Float);
        const LV2_URID ui_sampleRate = ui->itf.map->map(ui->itf.map->handle, LV2_PARAMETERS__sampleRate);
        for (const LV2_Options_Option* o = opts; o->key; ++o) {
            if (o->context == LV2_OPTIONS_INSTANCE &&
              o->key == ui_scaleFactor && o->type == atom_Float) {
                scale = *(float*)o->value;
                //break;
            } else if (o->context == LV2_OPTIONS_INSTANCE &&
              o->key == ui_sampleRate && o->type == atom_Float) {
                ui->uiKnowSampleRate = true;
                ui->uiSampleRate = (int)*(float*)o->value;
                //fprintf(stderr, "SampleRate = %iHz\n",(int)*(float*)o->value);
            }
        }
        if (scale > 1.0) ui->main.hdpi = scale;
    }

    int w = 1;
    int h = 1;
    plugin_set_window_size(&w,&h,plugin_uri);
    // create the toplevel Window on the parentXwindow provided by the host
    ui->win = create_window(&ui->main, (Window)ui->parentXwindow, 0, 0, w, h);
    ui->win->parent_struct = ui;
    // create controller widgets
    plugin_create_controller_widgets(ui,plugin_uri);
    // map all widgets into the toplevel Widget_t
    widget_show_all(ui->win);
    // set the widget pointer to the X11 Window from the toplevel Widget_t
    *widget = (void*)ui->win->widget;
    // request to resize the parentXwindow to the size of the toplevel Widget_t
    if (ui->itf.resize){
        ui->itf.resize->ui_resize(ui->itf.resize->handle, ui->win->width, ui->win->height);
    }
    // store pointer to the host controller
    ui->itf.controller = controller;
    // store pointer to the host write function
    ui->itf.write_function = write_function;
    
    return (LV2UI_Handle)ui;
}

// cleanup after usage
static void cleanup(LV2UI_Handle handle) {
    X11_UI* ui = (X11_UI*)handle;
    plugin_cleanup(ui);
    // Xputty free all memory used
    main_quit(&ui->main);
    free(ui->private_ptr);
    free(ui);
}

static void null_callback(void *w_, void* user_data) {
    
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                        LV2 interface
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

// port value change message from host
static void port_event(LV2UI_Handle handle, uint32_t port_index,
                        uint32_t buffer_size, uint32_t format,
                        const void * buffer) {
    X11_UI* ui = (X11_UI*)handle;
    float value = *(float*)buffer;
    int i=0;
    for (;i<CONTROLS;i++) {
        if (ui->widget[i] && port_index == (uint32_t)ui->widget[i]->data) {
            // prevent event loop between host and plugin
            xevfunc store = ui->widget[i]->func.value_changed_callback;
            ui->widget[i]->func.value_changed_callback = null_callback;
            // Xputty check if the new value differs from the old one
            // and set new one, when needed
            adj_set_value(ui->widget[i]->adj, value);
            // activate value_change_callback back
            ui->widget[i]->func.value_changed_callback = store;
        }
   }
   plugin_port_event(handle, port_index, buffer_size, format, buffer);
}

static void notify_dsp(X11_UI *ui, const X11LV2URIs* uris) {
    uint8_t obj_buf[OBJ_BUF_SIZE];
    lv2_atom_forge_set_buffer(&ui->itf.forge, obj_buf, OBJ_BUF_SIZE);
    LV2_Atom_Forge_Frame frame;
    LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&ui->itf.forge, &frame, 0, uris->patch_Get);

    ui->itf.write_function(ui->itf.controller, 5, lv2_atom_total_size(msg),
                       uris->atom_eventTransfer, msg);
}

void first_loop(X11_UI *ui) {
    const X11LV2URIs* uris = &ui->itf.uris;
    notify_dsp(ui, uris);
}

// LV2 idle interface to host
static int ui_idle(LV2UI_Handle handle) {
    X11_UI* ui = (X11_UI*)handle;
    if (ui->need_resize == 1) {
        ui->need_resize = 2;
    } else if (ui->need_resize == 2) {
        int i=0;
        for (;i<CONTROLS;i++) {
            os_move_window(ui->main.dpy, ui->widget[i], ui->widget[i]->x, ui->widget[i]->y);
        }
        ui->need_resize = 0;
    }
    // Xputty event loop setup to run one cycle when called
    run_embedded(&ui->main);
#ifdef USE_ATOM
    if (ui->loop_counter > 0) {
        ui->loop_counter--;
        if (ui->loop_counter == 0)
            first_loop(ui);
    }
#endif
    return 0;
}

// LV2 resize interface to host
static int ui_resize(LV2UI_Feature_Handle handle, int w, int h) {
    X11_UI* ui = (X11_UI*)handle;
    // Xputty sends configure event to the toplevel widget to resize itself
    if (ui) send_configure_event(ui->win,0, 0, w, h);
    return 0;
}

// connect idle and resize functions to host
static const void* extension_data(const char* uri) {
    static const LV2UI_Idle_Interface idle = { ui_idle };
    static const LV2UI_Resize resize = { 0 ,ui_resize };
    if (!strcmp(uri, LV2_UI__idleInterface)) {
        return &idle;
    }
    if (!strcmp(uri, LV2_UI__resize)) {
        return &resize;
    }
    return NULL;
}

static const LV2UI_Descriptor descriptors[] = {
    {PLUGIN_UI_URI,instantiate,cleanup,port_event,extension_data},
#ifdef PLUGIN_UI_URI2
    {PLUGIN_UI_URI2,instantiate,cleanup,port_event,extension_data},
#endif
};

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index) {
    if (index >= sizeof(descriptors) / sizeof(descriptors[0])) {
        return NULL;
    }
    return descriptors + index;
}

