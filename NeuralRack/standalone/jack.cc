/*
 * jack.cc
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */

#include <jack/jack.h>
#include <jack/thread.h>


/****************************************************************
        jack.cc   native jackd support for Ratatouille
        
        this file is meant to be included in main.
****************************************************************/

jack_client_t *client;
jack_port_t *in_port;
jack_port_t *out_port;
jack_port_t *out1_port;

void jack_shutdown (void *arg) {
    fprintf (stderr, "jack shutdown, exit now \n");
    r->quitGui();
}

int jack_xrun_callback(void *arg) {
    fprintf (stderr, "Xrun \r");
    return 0;
}

int jack_srate_callback(jack_nframes_t samplerate, void* arg) {
    int prio = jack_client_real_time_priority(client);
    if (prio < 0) prio = 25;
    fprintf (stderr, "Samplerate %iHz \n", samplerate);
    r->initEngine(samplerate, prio, 1);
    return 0;
}

int jack_buffersize_callback(jack_nframes_t nframes, void* arg) {
    fprintf (stderr, "Buffersize is %i samples \n", nframes);
    return 0;
}

int jack_process(jack_nframes_t nframes, void *arg) {
    float *input = static_cast<float *>(jack_port_get_buffer (in_port, nframes));
    float *output = static_cast<float *>(jack_port_get_buffer (out_port, nframes));
    float *output1 = static_cast<float *>(jack_port_get_buffer (out1_port, nframes));
    
    if(output != input)
        memcpy(output, input, nframes*sizeof(float));
    if(output1 != input)
        memcpy(output1, input, nframes*sizeof(float));

    r->process(nframes, output, output1);

    return 0;
}

void startJack() {

    if ((client = jack_client_open ("neuralrack", JackNoStartServer, NULL)) == 0) {
        fprintf (stderr, "jack server not running?\n");
        r->quitGui();
    }

    if (client) {
        in_port = jack_port_register(
                       client, "in_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        out_port = jack_port_register(
                       client, "out_0", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        out1_port = jack_port_register(
                       client, "out_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        jack_set_xrun_callback(client, jack_xrun_callback, 0);
        jack_set_sample_rate_callback(client, jack_srate_callback, 0);
        jack_set_buffer_size_callback(client, jack_buffersize_callback, 0);
        jack_set_process_callback(client, jack_process, 0);
        jack_on_shutdown (client, jack_shutdown, 0);

        if (jack_activate (client)) {
            fprintf (stderr, "cannot activate client");
            r->quitGui();
        }

        if (!jack_is_realtime(client)) {
            fprintf (stderr, "jack isn't running with realtime priority\n");
        } else {
            fprintf (stderr, "jack running with realtime priority\n");
        }
        r->enableEngine(1);
        r->readConfig();
    }
}

void quitJack() {
     if (client) {
        if (jack_port_connected(in_port)) jack_port_disconnect(client,in_port);
        jack_port_unregister(client,in_port);
        if (jack_port_connected(out_port)) jack_port_disconnect(client,out_port);
        jack_port_unregister(client,out1_port);
        if (jack_port_connected(out1_port)) jack_port_disconnect(client,out1_port);
        jack_port_unregister(client,out1_port);
        jack_client_close (client);
    }    
}
