/*
 * NeuralRack.cpp
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */

#include <clap.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "NeuralRack.cc"

#define WINDOW_WIDTH  620
#define WINDOW_HEIGHT 580

#if defined(_WIN32)
#define GUIAPI CLAP_WINDOW_API_WIN32
#else
#define GUIAPI CLAP_WINDOW_API_X11
#endif


/****************************************************************
 ** neuralrack_plugin_t -> the plugin struct
 */

// Plugin data structure
typedef struct {
    clap_plugin_t plugin;
    const clap_host_t *host;
    NeuralRack *r;
    bool guiIsCreated;
    uint32_t latency;
} neuralrack_plugin_t;


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


/****************************************************************
 ** define the audio ports
 */

// Audio Ports
static uint32_t audio_ports_count(const clap_plugin_t *plugin, bool is_input) {
    if (is_input) return 1; // 1 input
    else return 2; // and 2 output
}

static bool audio_ports_get(const clap_plugin_t *plugin, uint32_t index, bool is_input, clap_audio_port_info_t *info) {
    if (index > 0) return false;
    info->id = index;
    snprintf(info->name, sizeof(info->name), "%s", is_input ? "Input" : "Output");
    if (is_input) {
        info->channel_count = 1; // Mono
        info->port_type = CLAP_PORT_MONO;
    } else {
        info->channel_count = 2; // Stereo
        info->port_type = CLAP_PORT_STEREO;
    }
    info->flags = CLAP_AUDIO_PORT_IS_MAIN;
    return true;
}

static const clap_plugin_audio_ports_t audio_ports = {
    .count = audio_ports_count,
    .get = audio_ports_get,
};

/****************************************************************
 ** Latency reporting
 */

// Latency
static uint32_t neuralrack_latency_get(const clap_plugin_t *plugin) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    plug->r->getLatency(&plug->latency);
    return plug->latency;
}

static const clap_plugin_latency_t latency_extension = {
    .get = neuralrack_latency_get,
};

/****************************************************************
 ** save and load states
 */

// State Management
static bool neuralrack_state_save(const clap_plugin_t *plugin, const clap_ostream_t *stream) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    std::string state;
    plug->r->saveState(&state);
    stream->write(stream, state.c_str(), strlen(state.c_str()));
    return true;
}

static bool neuralrack_state_load(const clap_plugin_t *plugin, const clap_istream_t *stream) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    char state[2048];
    char *curr = state;
    int thisread = stream->read(stream, curr, 2048);
    if (thisread < 0) return false;
    std::string stream_  = state ;
    plug->r->readState(stream_);
    return true;
}

static const clap_plugin_state_t state_extension = {
    .save = neuralrack_state_save,
    .load = neuralrack_state_load,
};

/****************************************************************
 ** GUI handling
 */

// GUI Callbacks
static bool neuralrack_gui_is_api_supported(const clap_plugin *plugin, const char *api, bool is_floating) {
    return strcmp(api, GUIAPI) == 0;
}

static bool neuralrack_gui_get_preferred_api(const clap_plugin_t *plugin, const char **api, bool *isFloating) {
    *api = GUIAPI;
    *isFloating = false;
    return true;
}

static bool neuralrack_gui_set_scale(const clap_plugin_t *plugin, double scale) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    plug->r->getMain()->hdpi = scale;
    return true;
}

static bool neuralrack_gui_get_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    *width = WINDOW_WIDTH;
    *height = WINDOW_HEIGHT;
    return true;
}

static bool neuralrack_gui_can_resize(const clap_plugin_t *plugin) {
    return false;
}

static bool neuralrack_gui_get_resize_hints(const clap_plugin_t *plugin, clap_gui_resize_hints_t *hints) {
    return false;
}

static bool neuralrack_gui_adjust_size(const clap_plugin_t *plugin, uint32_t *width, uint32_t *height) {
    *width = WINDOW_WIDTH;
    *height = WINDOW_HEIGHT;
    return true;   
}

static bool neuralrack_gui_set_transient(const clap_plugin_t *plugin, const clap_window_t *window) {
    return false;
}

static void neuralrack_gui_suggest_title(const clap_plugin_t *plugin, const char *title) {
    title = "NeuralRack";
}

static bool neuralrack_gui_create(const clap_plugin *plugin, const char *api, bool is_floating) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    if (strcmp(api, GUIAPI) == 0) {
        if (!plug->guiIsCreated) {
            plug->r->startGui();
            plug->r->enableEngine(1);
        }
        plug->guiIsCreated = true;
        return true;
    }
    return false;
}

static void neuralrack_gui_destroy(const clap_plugin *plugin) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    plug->r->quitGui();
    plug->guiIsCreated = false;
}

static bool neuralrack_gui_show(const clap_plugin *plugin) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    plug->r->showGui();
    return true;
}

static bool neuralrack_gui_hide(const clap_plugin *plugin) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    plug->r->hideGui();
    return true;
}

// embed the GUI
static bool neuralrack_gui_set_parent(const clap_plugin_t *plugin, const clap_window_t *window) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    if (!plug->guiIsCreated) {
        plug->r->startGui(window);
        plug->r->enableEngine(1);
    }
    plug->guiIsCreated = true;
    plug->r->setParent(window);
    plug->r->showGui();
    return true;
}

static bool neuralrack_gui_set_size(const clap_plugin_t *plugin, uint32_t width, uint32_t height) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    os_resize_window(plug->r->getMain()->dpy, plug->r->TopWin, width, height);
    return true;
}

// Main thread callback (we run our own main thread)
static void neuralrack_on_main_thread(const clap_plugin_t *plugin) {
   // neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
   // plug->r->runGui();
}

static const clap_plugin_gui_t extensionGUI = {
    .is_api_supported = neuralrack_gui_is_api_supported,
    .get_preferred_api = neuralrack_gui_get_preferred_api,
    .create = neuralrack_gui_create,
    .destroy = neuralrack_gui_destroy,
    .set_scale = neuralrack_gui_set_scale,
    .get_size = neuralrack_gui_get_size,
    .can_resize = neuralrack_gui_can_resize,
    .get_resize_hints = neuralrack_gui_get_resize_hints,
    .adjust_size = neuralrack_gui_adjust_size,
    .set_size = neuralrack_gui_set_size,
    .set_parent = neuralrack_gui_set_parent,
    .set_transient = neuralrack_gui_set_transient,
    .suggest_title = neuralrack_gui_suggest_title,
    .show = neuralrack_gui_show,
    .hide = neuralrack_gui_hide,
};

/****************************************************************
 ** Plugin handling
 */

// Initialize the plugin
static bool neuralrack_init(const clap_plugin_t *plugin) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    plug->r->initEngine(48000, 25, 1);
    return true;
}

// Destroy the plugin
static void neuralrack_destroy(const clap_plugin_t *plugin) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    neuralrack_gui_destroy(plugin);
    delete plug->r;
    free(plug);
}

// Audio processing
static clap_process_status neuralrack_process(const clap_plugin_t *plugin, const clap_process_t *process) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    // Ensure there is one input and two outputs
    if (process->audio_inputs[0].channel_count < 1 || process->audio_outputs[0].channel_count < 2) {
        return false; // Invalid format
    }

    float *input = process->audio_inputs[0].data32[0]; // Mono input channel
    float *left_output = process->audio_outputs[0].data32[0]; // Left channel of stereo output
    float *right_output = process->audio_outputs[0].data32[1]; // Right channel of stereo output
    uint32_t nframes = process->frames_count;
    // in-place processing
    if(left_output != input)
        memcpy(left_output, input, nframes*sizeof(float));
    if(right_output != input)
        memcpy(right_output, input, nframes*sizeof(float));
    
    plug->r->process(nframes, left_output, right_output);
    return CLAP_PROCESS_CONTINUE;
}


// Finally get the sample rate and re-init the engine
static bool neuralrack_activate(const struct clap_plugin *plugin,
                             double                    sample_rate,
                             uint32_t                  min_frames_count,
                             uint32_t                  max_frames_count) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)plugin->plugin_data;
    //if (sample_rate != 48000) 
    plug->r->initEngine(sample_rate, 25, 1);
    return true;
}

static void neuralrack_deactivate(const struct clap_plugin *plugin) {}

static bool neuralrack_start_processing(const struct clap_plugin *plugin) { return true; }

static void neuralrack_stop_processing(const struct clap_plugin *plugin) {}

static void neuralrack_reset(const struct clap_plugin *plugin) {}

// CLAP plugin descriptor
static const clap_plugin_descriptor_t neuralrack_descriptor = {
    .clap_version = CLAP_VERSION_INIT,
    .id = "com.brummer10.NeuralRack",
    .name = "NeuralRack",
    .vendor = "brummer10",
    .url = "https://github.com/brummer10/NeuralRack",
    .manual_url = "https://github.com/brummer10/NeuralRack",
    .support_url = "https://github.com/brummer10/NeuralRack",
    .version = "1.0.0",
    .description = "CLAP plugin wrapper for NeuralRack",
    .features = (const char *[]){ CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, NULL },
};

// Extensions
static const void *neuralrack_get_extension(const clap_plugin_t *plugin, const char *id) {
    if (!strcmp(id, CLAP_EXT_AUDIO_PORTS)) return &audio_ports;
    if (!strcmp(id, CLAP_EXT_LATENCY)) return &latency_extension;
    if (!strcmp(id, CLAP_EXT_GUI)) return &extensionGUI;
    if (!strcmp(id, CLAP_EXT_STATE)) return &state_extension;
    return NULL;
}

// Create the plugin
static const clap_plugin_t *neuralrack_create(const clap_host_t *host) {
    neuralrack_plugin_t *plug = (neuralrack_plugin_t *)calloc(1, sizeof(neuralrack_plugin_t));
    if (!plug) return NULL;
    plug->r = new NeuralRack();
    plug->guiIsCreated = false;
    plug->plugin.desc = &neuralrack_descriptor;
    plug->plugin.plugin_data = plug;
    plug->plugin.init = neuralrack_init;
    plug->plugin.destroy = neuralrack_destroy;
    plug->plugin.activate = neuralrack_activate;
    plug->plugin.deactivate = neuralrack_deactivate;
    plug->plugin.start_processing = neuralrack_start_processing;
    plug->plugin.stop_processing = neuralrack_stop_processing;
    plug->plugin.reset = neuralrack_reset;
    plug->plugin.process = neuralrack_process;
    plug->plugin.get_extension = neuralrack_get_extension;
    plug->plugin.on_main_thread = neuralrack_on_main_thread;
    plug->host = host;
    return &plug->plugin;
}


/****************************************************************
 ** the factory entry
 */

static uint32_t plugin_factory_get_plugin_count(const struct clap_plugin_factory *factory) {
   return 1;
}

static const clap_plugin_descriptor_t *plugin_factory_get_neuralrack_descriptor
                    (const struct clap_plugin_factory *factory, uint32_t index) {
   return  &neuralrack_descriptor; //s_plugins[index].desc;
}

static const clap_plugin_t *plugin_factory_create_neuralrack
                        (const struct clap_plugin_factory *factory,
                        const clap_host_t *host, const char *plugin_id) {

   if (!clap_version_is_compatible(host->clap_version)) {
      return NULL;
   }
   return neuralrack_create(host);
}

static const clap_plugin_factory_t plugin_factory = {
    .get_plugin_count = plugin_factory_get_plugin_count,
    .get_plugin_descriptor = plugin_factory_get_neuralrack_descriptor,
    .create_plugin = plugin_factory_create_neuralrack,
};

static const void *entry_get_factory(const char *factory_id) {
    return &plugin_factory;
}

static bool entry_init(const char *plugin_path) {
   // perform the plugin initialization
   return true;
}

static void entry_deinit(void) {
   // perform the plugin de-initialization
}

/****************************************************************
 ** Finally the CLAP plugin entry export
 */

extern "C" const clap_plugin_entry_t clap_entry = {
    .clap_version = CLAP_VERSION_INIT,
    .init = entry_init,
    .deinit = entry_deinit,
    .get_factory = entry_get_factory,
};
