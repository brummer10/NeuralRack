/*
 * main.cpp
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */


#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h> 

#include <atomic>
#include <iostream>
#include <string>
#include <cmath>

#if defined(HAVE_PA)
#include "xpa.h"
#endif


#include "NeuralRack.cc"

NeuralRack *r;

#if defined(HAVE_JACK)
#include "jack.cc"
#endif

// send value changes from GUI to the engine
void sendValueChanged(X11_UI *ui, int port, float value) {
    r->sendValueChanged(port, value);
}

// send a file name from GUI to the engine
void sendFileName(X11_UI *ui, ModelPicker* m, int old){
    r->sendFileName(m, old);
}

// the portaudio server process callback
#if defined(HAVE_PA)
static int process(const void* inputBuffer, void* outputBuffer,
    unsigned long nframes, const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void* data) {

    const float* input = ((const float**)inputBuffer)[0];
    float* output = ((float**)outputBuffer)[0];
    float* output1 = ((float**)outputBuffer)[1];

    if(output != input)
        memcpy(output, input, nframes*sizeof(float));
    if(output1 != input)
        memcpy(output1, input, nframes*sizeof(float));

    r->process(nframes, output, output1);

    return 0;
}
#endif

#if defined(__linux__) || defined(__FreeBSD__) || \
    defined(__NetBSD__) || defined(__OpenBSD__)
void signal_handler (int sig) {
    switch (sig) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
        case SIGQUIT:
            fprintf (stderr, "\nsignal %i received, exiting ...\n", sig);
            r->quitGui();
        break;
        default:
        break;
    }
}
#endif

int main(int argc, char *argv[]){

    #if defined(__linux__) || defined(__FreeBSD__) || \
        defined(__NetBSD__) || defined(__OpenBSD__)
    #if defined(PAWPAW)
    setenv("FONTCONFIG_PATH", "/etc/fonts", true);
    #endif
    if(0 == XInitThreads()) 
        fprintf(stderr, "Warning: XInitThreads() failed\n");
    #endif
    #if defined(HAVE_PA)
    bool runPA = false;
    #endif
    r = new NeuralRack();
    r->startGui();

    #if defined(__linux__) || defined(__FreeBSD__) || \
        defined(__NetBSD__) || defined(__OpenBSD__)
    signal (SIGQUIT, signal_handler);
    signal (SIGTERM, signal_handler);
    signal (SIGHUP, signal_handler);
    signal (SIGINT, signal_handler);
    #endif

    #if defined(HAVE_PA)
    XPa xpa ("Neuralrack");
    if(!xpa.openStream(1, 2, &process, nullptr)) {
        #if defined(HAVE_JACK)
        startJack();
        #else    
        r->quitGui();
        #endif
    } else {
        runPA = true;
        r->initEngine(xpa.getSampleRate(), 25, 1);
        r->enableEngine(1);
        r->readConfig();
        if(!xpa.startStream()) r->quitGui();
        r->setXPa(&xpa, xpa.haveASIO);
    }
    #else
    startJack();
    #endif

    main_run(r->getMain());

    #if defined(HAVE_PA)
    if (runPA) xpa.stopStream();
    #if defined(HAVE_JACK)
    else quitJack();
    #endif
    #else
    quitJack();
    #endif

    r->cleanup();
    delete r;

    printf("bye bye\n");
    return 0;
}

