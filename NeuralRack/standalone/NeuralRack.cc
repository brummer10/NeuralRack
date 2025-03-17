/*
 * NeuralRack.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */


#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include <atomic>
#include <iostream>
#include <cstring>
#include <string>
#include <cmath>

#include <locale.h>

#include "engine.h"
#include "ParallelThread.h"
#define STANDALONE
#include "NeuralRack.c"

class NeuralRack
{
public:

    NeuralRack() : engine() {
        workToDo.store(false, std::memory_order_release);
        processCounter = 0;
        settingsHaveChanged = false;
        s_time = 0.0;
        ui = (X11_UI*)malloc(sizeof(X11_UI));
        ui->private_ptr = NULL;
        ui->need_resize = 1;
        ui->loop_counter = 4;
        ui->uiKnowSampleRate = false;
        ui->uiSampleRate = 0;
        ui->f_index = 0;
        for(int i = 0;i<CONTROLS;i++)
            ui->widget[i] = NULL;
        getConfigFilePath();
    }

    ~NeuralRack() {
        fetch.stop();
        free(ui->private_ptr);
        free(ui);
        //cleanup();
    }

    void startGui() {
        main_init(&ui->main);
        set_custom_theme(ui);
        ui->win = create_window(&ui->main, os_get_root_window(&ui->main, IS_WINDOW), 0, 0, 610, 510);
        widget_set_title(ui->win, "NeuralRack");
        widget_set_icon_from_png(ui->win,LDVAR(neuraldir_png));
        ui->win->parent_struct = ui;
        plugin_create_controller_widgets(ui,"standalone");
        widget_show_all(ui->win);
    }

    Xputty *getMain() {
        return &ui->main;
    }

    void quitGui() {
        #if defined(__linux__) || defined(__FreeBSD__) || \
            defined(__NetBSD__) || defined(__OpenBSD__)
        XLockDisplay(ui->main.dpy);
        #endif
        destroy_widget(ui->win, &ui->main);
         #if defined(__linux__) || defined(__FreeBSD__) || \
            defined(__NetBSD__) || defined(__OpenBSD__)
        XFlush(ui->main.dpy);
        XUnlockDisplay(ui->main.dpy);
        #endif
   }

    void initEQ() {
        engine.eqOnOff = 0;
        engine.peq->fVslider1 =  -20.0;
        engine.peq->fVslider0 =  0.0;
        engine.peq->fVslider2 =  0.0;
        engine.peq->fVslider3 =  0.0;
        engine.peq->fVslider4 =  0.0;
        engine.peq->fVslider5 =  -20.0;
    }

    void initEngine(uint32_t rate, int32_t prio, int32_t policy) {
        engine.init(rate, prio, policy);
        initEQ();
        s_time = (1.0 / (double)rate) * 1000;
        fetch.startTimeout(120);
        fetch.set<NeuralRack, &NeuralRack::checkEngine>(this);
    }

    void enableEngine(int on) {
        adj_set_value(ui->widget[10]->adj, static_cast<float>(on));
    }

    inline void process(uint32_t n_samples, float* output, float* output1) {
        engine.process(n_samples, output, output1);
    }

    // send value changes from GUI to the engine
    void sendValueChanged(int port, float value) {
        settingsHaveChanged = true;
        switch (port) {
            // 0 + 1 audio ports
            case 2:
                engine.inputGain = value;
            break;
            case 3:
                engine.outputGain = value;
            break;
            case 4:
                engine.outputGain1 = value;
            break;
            // 5 + 6 atom ports
            case 7:
                engine.IRoutputGain = value;
            break;
            case 8:
                engine.IRoutputGain1 = value;
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
            break;
            case 12:
                engine.normSlotA = static_cast<int32_t>(value);
            break;
            case 13:
                engine.normSlotB = static_cast<int32_t>(value);
            break;
            case 14:
                engine.bypass = static_cast<int32_t>(value);
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
            }
            break;
            case 24:
                engine.peq->fVslider1 =  value;
            break;
            case 25:
                engine.peq->fVslider0 =  value;
            break;
            case 26:
                engine.peq->fVslider2 =  value;
            break;
            case 27:
                engine.peq->fVslider3 =  value;
            break;
            case 28:
                engine.peq->fVslider4 =  value;
            break;
            case 29:
                engine.peq->fVslider5 =  value;
            break;
            case 30:
                engine.eqOnOff = static_cast<int32_t>(value);

            break;
            default:
            break;
        }
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
        } else if (ends_with(m->filename, "wav")) {
            if ( m == &ps->ir) {
                engine.ir_file = m->filename;
                engine._cd.fetch_add(1, std::memory_order_relaxed);
            } else {
                engine.ir_file1 = m->filename;
                engine._cd.fetch_add(2, std::memory_order_relaxed);
            }
        } else return;
        settingsHaveChanged = true;
        workToDo.store(true, std::memory_order_release);
    }

    void readConfig(std::string name = "Default") {
        try {
            std::ifstream infile(configFile);
            std::string line;
            std::string key;
            std::string value;
            std::string LoadName = "None";
            if (infile.is_open()) {
                infile.imbue (std::locale("C"));
                while (std::getline(infile, line)) {
                    std::istringstream buf(line);
                    buf >> key;
                    buf >> value;
                    if (key.compare("[Preset]") == 0) LoadName = remove_sub(line, "[Preset] ");
                    if (name.compare(LoadName) == 0) {
                        if (key.compare("[CONTROLS]") == 0) {
                            for (int i = 0; i < CONTROLS; i++) {
                                adj_set_value(ui->widget[i]->adj, check_stod(value));
                                buf >> value;
                            }
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
                    }
                    key.clear();
                    value.clear();
                }
                infile.close();
                workToDo.store(true, std::memory_order_release);
            }
        } catch (std::ifstream::failure const&) {
            return;
        }
    }

    void cleanup() {
        fetch.stop();
        if (settingsHaveChanged)
            saveConfig();
        plugin_cleanup(ui);
        // Xputty free all memory used
        main_quit(&ui->main);
    }

private:
    ParallelThread          fetch;
    X11_UI*                 ui;
    neuralrack::Engine     engine;
    int                     processCounter;
    bool                    settingsHaveChanged;
    std::atomic<bool>       workToDo;
    std::string             configFile;
    double                  s_time;

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

    void getConfigFilePath() {
         if (getenv("XDG_CONFIG_HOME")) {
            std::string path = getenv("XDG_CONFIG_HOME");
            configFile = path + "/neuralrack.conf";
        } else {
        #if defined(__linux__) || defined(__FreeBSD__) || \
            defined(__NetBSD__) || defined(__OpenBSD__)
            std::string path = getenv("HOME");
            configFile = path +"/.config/neuralrack.conf";
        #else
            std::string path = getenv("APPDATA");
            configFile = path +"\\.config\\neuralrack.conf";
        #endif
       }
    }

    void saveConfig(std::string name = "Default",  bool append = false) {
        std::ofstream outfile(configFile, append ? std::ios::app : std::ios::trunc);
        if (outfile.is_open()) {
            outfile.imbue (std::locale("C"));
            outfile << "[Preset] " << name << std::endl;
            outfile << "[CONTROLS] ";
            for(int i = 0;i<CONTROLS;i++) {
                outfile << adj_get_value(ui->widget[i]->adj) << " ";
            }
            outfile << std::endl;
            outfile << "[Model] " << engine.model_file << std::endl;
            outfile << "[Model1] " << engine.model_file1 << std::endl;
            outfile << "[IrFile] " << engine.ir_file << std::endl;
            outfile << "[IrFile1] " << engine.ir_file1 << std::endl;
            outfile.close();
        }
    }

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

    // timeout loop to check output ports from engine
    void checkEngine() {
        // come back later
        if (processCounter < 2) {
            processCounter++;
            return;
        }
        if (workToDo.load(std::memory_order_acquire)) {
            if (engine.xrworker.getProcess()) {
                workToDo.store(false, std::memory_order_release);
                engine._execute.store(true, std::memory_order_release);
                engine.xrworker.runProcess();
            }
        } else if (engine._notify_ui.load(std::memory_order_acquire)) {
            engine._notify_ui.store(false, std::memory_order_release);
            #if defined(__linux__) || defined(__FreeBSD__) || \
                defined(__NetBSD__) || defined(__OpenBSD__)
            XLockDisplay(ui->main.dpy);
            #endif
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
            #if defined(__linux__) || defined(__FreeBSD__) || \
                defined(__NetBSD__) || defined(__OpenBSD__)
            XFlush(ui->main.dpy);
            XUnlockDisplay(ui->main.dpy);
            #endif
        }
    }

};
