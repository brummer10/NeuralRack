/*
 * NeuralRack.cc
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */


#include <atomic>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cmath>

#include <locale.h>

#include "engine.h"
#include "ParallelThread.h"
#include "Parameter.h"
#define CLAPPLUG
#include "NeuralRack.c"

class NeuralRack 
{
public:
    Widget_t*               TopWin;
    Params                  param;

    NeuralRack() : engine(), param() {
        workToDo.store(false, std::memory_order_release);
        s_time = 0.0;
        ui = (X11_UI*)malloc(sizeof(X11_UI));
        ui->private_ptr = NULL;
        ui->need_resize = 1;
        ui->loop_counter = 4;
        ui->uiKnowSampleRate = false;
        ui->setVerbose = false;
        ui->uiSampleRate = 0;
        ui->f_index = 0;
        title = "NeuralRack";
        firstLoop = true;
        p = 0;
        registerParameters();
        for(int i = 0;i<CONTROLS;i++)
            ui->widget[i] = NULL;
    }

    ~NeuralRack() {
        fetch.stop();
        free(ui->private_ptr);
        free(ui);
    }

    void registerParameters() {
        //                  name             group   min, max, def, step   value              isStepped  type
        param.registerParam("Buffered Mode",  "Global", 0,2,0,1,     (void*)&engine.buffered,      true,  Is_FLOAT);
        param.registerParam("Enable",         "Global", 0,1,1,1,     (void*)&engine.bypass,        true,  IS_UINT);

        param.registerParam("Gate Enable",    "NoiseGate", 0,1,0,1,  (void*)&engine.ngOnOff,       true,  IS_UINT);
        param.registerParam("Gate Thresh",    "NoiseGate", 0.01,0.31,0.017,0.001, (void*)&engine.ngate->threshold, false, Is_FLOAT);

        param.registerParam("Norm Slot A",    "Pedal",  0,1,0,1,     (void*)&engine.normSlotA,      true,  IS_INT);
        param.registerParam("Input Gain A",   "Pedal", -20,20,0,0.1, (void*)&engine.inputGain,      false, Is_FLOAT);
        param.registerParam("Output Gain A",  "Pedal", -20,20,0,0.1, (void*)&engine.outputGain,     false, Is_FLOAT);

        param.registerParam("EQ Enable",      "EQ",     0,1,0,1,     (void*)&engine.eqOnOff,        true,  IS_UINT);
        param.registerParam("EQ Band 1",      "EQ",    -20,20,0,0.1, (void*)&engine.peq->fVslider1, false, Is_FLOAT);
        param.registerParam("EQ Band 2",      "EQ",    -20,20,0,0.1, (void*)&engine.peq->fVslider0, false, Is_FLOAT);
        param.registerParam("EQ Band 3",      "EQ",    -20,20,0,0.1, (void*)&engine.peq->fVslider2, false, Is_FLOAT);
        param.registerParam("EQ Band 4",      "EQ",    -20,20,0,0.1, (void*)&engine.peq->fVslider3, false, Is_FLOAT);
        param.registerParam("EQ Band 5",      "EQ",    -20,20,0,0.1, (void*)&engine.peq->fVslider4, false, Is_FLOAT);
        param.registerParam("EQ Band 6",      "EQ",    -20,20,0,0.1, (void*)&engine.peq->fVslider5, false, Is_FLOAT);

        param.registerParam("Norm Slot B",    "Amp",    0,1,0,1,     (void*)&engine.normSlotB,      true,  IS_INT);
        param.registerParam("Input Gain B",   "Amp",   -20,20,0,0.1, (void*)&engine.inputGain1,     false, Is_FLOAT);
        param.registerParam("Output Gain B",  "Amp",   -20,20,0,0.1, (void*)&engine.outputGain1,    false, Is_FLOAT);

        param.registerParam("IR Out Gain L",  "IR",    -20,20,0,0.1, (void*)&engine.IRoutputGain,   false, Is_FLOAT);
        param.registerParam("IR Out Gain R",  "IR",    -20,20,0,0.1, (void*)&engine.IRoutputGain1,  false, Is_FLOAT);
    }

    void startGui(Window window) {
        main_init(&ui->main);
        set_custom_theme(ui);
        int w = 1;
        int h = 1;
        plugin_set_window_size(&w,&h,"clap_plugin");
        #if defined(_WIN32)
        TopWin  = create_window(&ui->main, (HWND) window, 0, 0, w, h);
        #else
        TopWin  = create_window(&ui->main, (Window) window, 0, 0, w, h);
        #endif
        TopWin->flags |= HIDE_ON_DELETE;
        ui->win = create_widget(&ui->main, TopWin, 0, 0, w, h);
        ui->win->scale.gravity = NORTHWEST;
        widget_set_title(TopWin, title.c_str());
        ui->win->parent_struct = ui;
        ui->win->private_struct = (void*)this;
        plugin_create_controller_widgets(ui,"clap_plugin");
        fetch.startTimeout(60);
        fetch.set<NeuralRack, &NeuralRack::runGui>(this);
    }

    void startGui() {
        main_init(&ui->main);
        set_custom_theme(ui);
        int w = 1;
        int h = 1;
        plugin_set_window_size(&w,&h,"clap_plugin");
        TopWin  = create_window(&ui->main, os_get_root_window(&ui->main, IS_WINDOW), 0, 0, w, h);
        TopWin->flags |= HIDE_ON_DELETE;
        ui->win = create_widget(&ui->main, TopWin, 0, 0, w, h);
        widget_set_title(TopWin, title.c_str());
        ui->win->parent_struct = ui;
        ui->win->private_struct = (void*)this;
        plugin_create_controller_widgets(ui,"clap_plugin");
        fetch.startTimeout(60);
        fetch.set<NeuralRack, &NeuralRack::runGui>(this);
    }

    void showGui() {
        engine._notify_ui.store(true, std::memory_order_release);
        getEngineValues();
        widget_show_all(TopWin);
        firstLoop = true;
    }
    
    void setParent(Window window) {
        #if defined(_WIN32)
        SetParent(TopWin->widget, (HWND) window);
        #else
        XReparentWindow(ui->main.dpy, TopWin->widget, (Window) window, 0, 0);
        #endif
        p = window;
    }

    void checkParentWindowSize(int width, int height) {
        #if defined (IS_VST2)
        if (!p) return;
        int host_width = 1;
        int host_height = 1;
        #if defined(_WIN32)
        RECT rect;
        if (GetClientRect((HWND) p, &rect)) {
            host_width  = rect.right - rect.left;
            host_height = rect.bottom - rect.top;
        }
        #else
        XWindowAttributes attrs;
        if (XGetWindowAttributes(ui->main.dpy, p, &attrs)) {
            host_width  = attrs.width;
            host_height = attrs.height;
        }
        #endif
        if ((host_width != width && host_width != 1) ||
            (host_height != height && host_height != 1)) {
            os_resize_window(ui->main.dpy, TopWin, host_width, host_height);
        }
        #endif
    }

    void hideGui() {
        widget_hide(TopWin);
        firstLoop = false;
    }

    void quitGui() {
        fetch.stop();
        cleanup();
        main_quit(&ui->main);
    }

    void runGui() {
        checkEngine();
        if (firstLoop) {
            checkParentWindowSize(TopWin->width, TopWin->height);
            firstLoop = false;
        }
        if (param.paramChanged.load(std::memory_order_acquire)) {
            getEngineValues();
            param.paramChanged.store(false, std::memory_order_release);
        }
        run_embedded(&ui->main);
    }

    // check output ports from engine
    void checkEngine() {
        
        if (workToDo.load(std::memory_order_acquire)) {
            if (engine.xrworker.getProcess()) {
                workToDo.store(false, std::memory_order_release);
                engine._execute.store(true, std::memory_order_release);
                engine.xrworker.runProcess();
            }
        } else if (engine._notify_ui.load(std::memory_order_acquire)) {
            engine._notify_ui.store(false, std::memory_order_release);
            X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;
            get_file(engine.model_file, &ps->ma);
            get_file(engine.model_file1, &ps->mb);
            get_file(engine.ir_file, &ps->ir);
            get_file(engine.ir_file1, &ps->ir1);
            adj_set_value(ui->widget[16]->adj,(float) engine.latency * s_time);
            adj_set_value(ui->widget[17]->adj,(float) engine.XrunCounter);
            expose_widget(ui->win);
            engine._ab.store(0, std::memory_order_release);
            engine._cd.store(0, std::memory_order_release);
        }
    }

    Xputty *getMain() {
        return &ui->main;
    }

    neuralrack::Engine *getEngine() {
        return &engine;
    }

    void initEQ() {
        engine.eqOnOff = 0;
        engine.peq->fVslider1 =  -20.0;
        engine.peq->fVslider0 =  0.0;
        engine.peq->fVslider2 =  0.0;
        engine.peq->fVslider3 =  0.0;
        engine.peq->fVslider4 =  0.0;
        engine.peq->fVslider5 =  -20.0;
        engine.ngate->threshold = 0.017;
        engine.ngOnOff = 0;
    }

    void initEngine(uint32_t rate, int32_t prio, int32_t policy) {
        engine.init(rate, prio, policy);
        initEQ();
        engine.bypass = 1;
        param.setParamDirty(1 , true);
        param.controllerChanged.store(true, std::memory_order_release);
        s_time = (1.0 / (double)rate) * 1000;
    }

    void enableEngine(int on) {
        adj_set_value(ui->widget[10]->adj, static_cast<float>(on));
    }

    inline void process(uint32_t n_samples, float* output, float* output1) {
        engine.process(n_samples, output, output1);
    }

    void getLatency(uint32_t* latency) {
        (*latency) = static_cast<uint32_t>(engine.latency);
    }

    void getEngineValues() {
        adj_set_value(ui->widget[0]->adj, engine.inputGain);
        adj_set_value(ui->widget[1]->adj, engine.outputGain);
        adj_set_value(ui->widget[2]->adj, engine.outputGain1);
        adj_set_value(ui->widget[3]->adj, engine.IRoutputGain);
        adj_set_value(ui->widget[4]->adj, engine.IRoutputGain1);
        adj_set_value(ui->widget[5]->adj, static_cast<float>(engine.conv.get_normalisation()));
        adj_set_value(ui->widget[6]->adj, static_cast<float>(engine.conv1.get_normalisation()));
        adj_set_value(ui->widget[7]->adj, engine.inputGain1);
        adj_set_value(ui->widget[8]->adj, static_cast<float>(engine.normSlotA));
        adj_set_value(ui->widget[9]->adj, static_cast<float>(engine.normSlotB));
        adj_set_value(ui->widget[10]->adj, static_cast<float>(engine.bypass));
        adj_set_value(ui->widget[15]->adj, engine.buffered);
        adj_set_value(ui->widget[18]->adj, engine.peq->fVslider1);
        adj_set_value(ui->widget[19]->adj, engine.peq->fVslider0);
        adj_set_value(ui->widget[20]->adj, engine.peq->fVslider2);
        adj_set_value(ui->widget[21]->adj, engine.peq->fVslider3);
        adj_set_value(ui->widget[22]->adj, engine.peq->fVslider4);
        adj_set_value(ui->widget[23]->adj, engine.peq->fVslider5);
        adj_set_value(ui->widget[24]->adj, engine.eqOnOff);
        adj_set_value(ui->widget[25]->adj, engine.ngate->threshold);
        adj_set_value(ui->widget[26]->adj, engine.ngOnOff);
    }

    // send value changes from GUI to the engine
    void sendValueChanged(int port, float value) {
        switch (port) {
            // 0 + 1 audio ports
            case 2:
                engine.inputGain = value;
                param.setParamDirty(5 , true);
            break;
            case 3:
                engine.outputGain = value;
                param.setParamDirty(6 , true);
            break;
            case 4:
                engine.outputGain1 = value;
                param.setParamDirty(16 , true);
            break;
            // 5 + 6 atom ports
            case 7:
                engine.IRoutputGain = value;
                param.setParamDirty(17 , true);
            break;
            case 8:
                engine.IRoutputGain1 = value;
                param.setParamDirty(18 , true);
            break;
            case 9:
            {
                engine._cd.fetch_add(1, std::memory_order_relaxed);
                engine.conv.set_normalisation(static_cast<uint32_t>(value));
                if (engine.ir_file.compare("None") != 0) {
                    workToDo.store(true, std::memory_order_release);
                }
            }
            break;
            case 10:
            {
                engine._cd.fetch_add(2, std::memory_order_relaxed);
                engine.conv1.set_normalisation(static_cast<uint32_t>(value));
                if (engine.ir_file1.compare("None") != 0) {
                    workToDo.store(true, std::memory_order_release);
                }
            }
            break;
            case 11:
                engine.inputGain1 = value;
                param.setParamDirty(15 , true);
            break;
            case 12:
                engine.normSlotA = static_cast<int32_t>(value);
                param.setParamDirty(4 , true);
            break;
            case 13:
                engine.normSlotB = static_cast<int32_t>(value);
                param.setParamDirty(14 , true);
            break;
            case 14:
                engine.bypass = static_cast<uint32_t>(value);
                param.setParamDirty(1 , true);
            break;
            case 15:
            {
                engine._ab.fetch_add(1, std::memory_order_relaxed);
                engine.model_file = "None";
                workToDo.store(true, std::memory_order_release);
            }
            break;
            case 16:
            {
                engine._ab.fetch_add(2, std::memory_order_relaxed);
                engine.model_file1 = "None";
                workToDo.store(true, std::memory_order_release);
             }
            break;
            case 17:
            {
                engine._cd.fetch_add(1, std::memory_order_relaxed);
                engine.ir_file = "None";
                workToDo.store(true, std::memory_order_release);
            }
            break;
            case 18:
            {
                engine._cd.fetch_add(2, std::memory_order_relaxed);
                engine.ir_file1 = "None";
                workToDo.store(true, std::memory_order_release);
            }
            break;
            // 19 latency
            case 20:
            {
                engine.buffered = value;
                engine._notify_ui.store(true, std::memory_order_release);
                param.setParamDirty(0 , true);
            }
            break;
            case 24:
                engine.peq->fVslider1 =  value;
                param.setParamDirty(8 , true);
            break;
            case 25:
                engine.peq->fVslider0 =  value;
                param.setParamDirty(9 , true);
            break;
            case 26:
                engine.peq->fVslider2 =  value;
                param.setParamDirty(10 , true);
            break;
            case 27:
                engine.peq->fVslider3 =  value;
                param.setParamDirty(11 , true);
            break;
            case 28:
                engine.peq->fVslider4 =  value;
                param.setParamDirty(12 , true);
            break;
            case 29:
                engine.peq->fVslider5 =  value;
                param.setParamDirty(13 , true);
            break;
            case 30:
                engine.eqOnOff = static_cast<uint32_t>(value);
                param.setParamDirty(7 , true);
            break;
            case 31:
                engine.ngate->threshold =  value;
                param.setParamDirty(3 , true);
            break;
            case 32:
                engine.ngOnOff = static_cast<uint32_t>(value);
                param.setParamDirty(2 , true);
            break;
            default:
            break;
        }
        // inform the process thread that a controller value was changed by the GUI thread
        param.controllerChanged.store(true, std::memory_order_release);
    }

    // send a file name from GUI to the engine
    void sendFileName(ModelPicker* m, int old) {
        X11_UI_Private_t *ps = (X11_UI_Private_t*)ui->private_ptr;
        if ((strcmp(m->filename, "None") == 0)) {
            if (old == 1) {
                if ( m == &ps->ma) {
                    engine.model_file = m->filename;
                    engine._ab.fetch_add(1, std::memory_order_relaxed);
                } else {
                    engine.model_file1 = m->filename;
                    engine._ab.fetch_add(2, std::memory_order_relaxed);
                }
            } else if (old == 2) {
                if ( m == &ps->ir) {
                    engine.ir_file = m->filename;
                    engine._cd.fetch_add(1, std::memory_order_relaxed);
                } else {
                    engine.ir_file1 = m->filename;
                    engine._cd.fetch_add(2, std::memory_order_relaxed);
                }
            } else return;
        } else if (ends_with(m->filename, "nam") ||
                   ends_with(m->filename, "json") ||
                   ends_with(m->filename, "aidax")) {
            if ( m == &ps->ma) {
                engine.model_file = m->filename;
                engine._ab.fetch_add(1, std::memory_order_relaxed);
            } else {
                engine.model_file1 = m->filename;
                engine._ab.fetch_add(2, std::memory_order_relaxed);
            }
        } else if (ends_with(m->filename, "wav")||
                   ends_with(m->filename, "WAV") ) {
            if ( m == &ps->ir) {
                engine.ir_file = m->filename;
                engine._cd.fetch_add(1, std::memory_order_relaxed);
            } else {
                engine.ir_file1 = m->filename;
                engine._cd.fetch_add(2, std::memory_order_relaxed);
            }
        } else return;
        workToDo.store(true, std::memory_order_release);
    }


    float check_stod (const std::string& str) {
        char* point = localeconv()->decimal_point;
        if (std::string(".") != point) {
            std::string::size_type point_it = str.find(".");
            std::string temp_str = str;
            if (point_it != std::string::npos)
                temp_str.replace(point_it, point_it + 1, point);
            return std::stod(temp_str);
        } else return std::stod(str);
    }

    std::string remove_sub(std::string a, std::string b) {
        std::string::size_type fpos = a.find(b);
        if (fpos != std::string::npos )
            a.erase(a.begin() + fpos, a.begin() + fpos + b.length());
        return (a);
    }

    void readState(std::string _stream) {
        std::string stream = _stream;
        std::string line;
        std::string key;
        std::string value;
        std::size_t pos = _stream.find("|");
        while (pos != std::string::npos) {
            line = stream.substr(0, pos);
            std::istringstream buf(line);
            buf >> key;
            buf >> value;
            if (key.compare("[CONTROLS]") == 0) {
                engine.inputGain = check_stod(value);
                buf >> value;
                engine.outputGain = check_stod(value);
                buf >> value;
                engine.outputGain1 = check_stod(value);
                buf >> value;
                engine.IRoutputGain = check_stod(value);
                buf >> value;
                engine.IRoutputGain1 = check_stod(value);
                buf >> value;
                engine.conv.set_normalisation((int)check_stod(value));
                buf >> value;
                engine.conv1.set_normalisation((int)check_stod(value));
                buf >> value;
                engine.inputGain1 = check_stod(value);
                buf >> value;
                engine.normSlotA = static_cast<int32_t>(check_stod(value));
                buf >> value;
                engine.normSlotB = static_cast<int32_t>(check_stod(value));
                buf >> value;
                engine.bypass = static_cast<uint32_t>(check_stod(value));
                buf >> value;
                engine.buffered = check_stod(value);
                buf >> value;
                engine.peq->fVslider1 = check_stod(value);
                buf >> value;
                engine.peq->fVslider0 = check_stod(value);
                buf >> value;
                engine.peq->fVslider2 = check_stod(value);
                buf >> value;
                engine.peq->fVslider3 = check_stod(value);
                buf >> value;
                engine.peq->fVslider4 = check_stod(value);
                buf >> value;
                engine.peq->fVslider5 = check_stod(value);
                buf >> value;
                engine.eqOnOff = static_cast<uint32_t>(check_stod(value));
                buf >> value;
                engine.ngate->threshold = check_stod(value);
                buf >> value;
                engine.ngOnOff = static_cast<uint32_t>(check_stod(value));
            } else if (key.compare("[Model]") == 0) {
                engine.model_file = remove_sub(line, "[Model] ");
                engine._ab.fetch_add(1, std::memory_order_relaxed);
            } else if (key.compare("[Model1]") == 0) {
                engine.model_file1 = remove_sub(line, "[Model1] ");
                engine._ab.fetch_add(2, std::memory_order_relaxed);
            } else if (key.compare("[IrFile]") == 0) {
                engine.ir_file = remove_sub(line, "[IrFile] ");
                engine._cd.fetch_add(1, std::memory_order_relaxed);
            } else if (key.compare("[IrFile1]") == 0) {
                engine.ir_file1 = remove_sub(line, "[IrFile1] ");
                engine._cd.fetch_add(2, std::memory_order_relaxed);
            }
            key.clear();
            value.clear();
            stream = stream.substr(pos+1);
            pos = stream.find("|");
            if (pos == std::string::npos) break;
        }
        workToDo.store(true, std::memory_order_release);
        if (engine.xrworker.getProcess()) {
            workToDo.store(false, std::memory_order_release);
            engine._execute.store(true, std::memory_order_release);
            engine.xrworker.runProcess();
        } else {
            fprintf(stderr, "Fail to get worker!!\n");
        }
    }

    void saveState(std::string *state) {
        std::ostringstream buffer; 
        buffer << "[CONTROLS] ";
        buffer << engine.inputGain << " ";
        buffer << engine.outputGain << " ";
        buffer << engine.outputGain1 << " ";
        buffer << engine.IRoutputGain << " ";
        buffer << engine.IRoutputGain1 << " ";
        buffer << engine.conv.get_normalisation() << " ";
        buffer << engine.conv1.get_normalisation() << " ";
        buffer << engine.inputGain1 << " ";
        buffer << engine.normSlotA << " ";
        buffer << engine.normSlotB << " ";
        buffer << engine.bypass << " ";
        buffer << engine.buffered << " ";
        buffer << engine.peq->fVslider1 << " ";
        buffer << engine.peq->fVslider0 << " ";
        buffer << engine.peq->fVslider2 << " ";
        buffer << engine.peq->fVslider3 << " ";
        buffer << engine.peq->fVslider4 << " ";
        buffer << engine.peq->fVslider5 << " ";
        buffer << engine.eqOnOff << " ";
        buffer << engine.ngate->threshold << " ";
        buffer << engine.ngOnOff << " ";
        buffer << "|";
        buffer << "[Model] " << engine.model_file << "|";
        buffer << "[Model1] " << engine.model_file1 << "|";
        buffer << "[IrFile] " << engine.ir_file << "|";
        buffer << "[IrFile1] " << engine.ir_file1 << "|";
        (*state) = buffer.str();
    }

    void cleanup() {
        plugin_cleanup(ui);
        free(ui->private_ptr);
        ui->private_ptr = NULL;
        // Xputty free all memory used
        // main_quit(&ui->main);
    }

private:
    ParallelThread          fetch;
    X11_UI*                 ui;
    neuralrack::Engine      engine;
    Window                  p;
    std::atomic<bool>       workToDo;
    double                  s_time;
    std::string             title;
    bool                    firstLoop;

    // rebuild file menu when needed
    void rebuild_file_menu(ModelPicker *m) {
        xevfunc store = m->fbutton->func.value_changed_callback;
        m->fbutton->func.value_changed_callback = dummy_callback;
        combobox_delete_entrys(m->fbutton);
        fp_get_files(m->filepicker, m->dir_name, 0, 1);
        int active_entry = m->filepicker->file_counter-1;
        for(uint32_t i = 0;i<m->filepicker->file_counter;i++) {
            combobox_add_entry(m->fbutton, m->filepicker->file_names[i]);
            if (strcmp(basename(m->filename),m->filepicker->file_names[i]) == 0) 
                active_entry = i;
        }
        combobox_add_entry(m->fbutton, "None");
        adj_set_value(m->fbutton->adj, active_entry);
        combobox_set_menu_size(m->fbutton, min(14, m->filepicker->file_counter+1));
        m->fbutton->func.value_changed_callback = store;
    }

    // confirmation from engine that a file is loaded
    inline void get_file(std::string fileName, ModelPicker *m) {
        if (!fileName.empty() && (fileName.compare("None") != 0)) {
            const char* uri = fileName.c_str();
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
            }
        } else if (strcmp(m->filename, "None") != 0) {
            free(m->filename);
            m->filename = NULL;
            m->filename = strdup("None");
        }
    }

};


/****************************************************************
 ** connect value change messages from the GUI (C) to the engine (C++)
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

