
/*
 * TextEntry.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */

/****************************************************************
        TextEntry - generate a text entry window 
                    and forward the input to the caller
****************************************************************/

#include "xwidgets.h"


#pragma once

#ifndef TEXTENTRY_H
#define TEXTENTRY_H

class TextEntry
{
public:

/****************************************************************
                Create and show a text entry window
****************************************************************/

    Widget_t *showTextEntry(Widget_t * w, const char* title, const char* message) {
        Widget_t *wid = create_window(w->app, os_get_root_window(w->app, IS_WINDOW), 0, 0,280, 70);
        wid->label = message;
        wid->scale.gravity = CENTER;
        wid->parent = w;
        widget_set_title(wid, title);
        widget_set_icon_from_png(wid,LDVAR(NeuralRack_png));
        wid->func.expose_callback = draw_window;
        os_set_transient_for_hint(w, wid);
        Widget_t *textentry = create_widget(w->app, wid, 20, 20, 200, 30);
        memset(textentry->input_label, 0, 32 * (sizeof textentry->input_label[0]) );
        textentry->func.expose_callback = entry_add_text;
        textentry->func.key_press_callback = entry_get_text;
        textentry->flags &= ~USE_TRANSPARENCY;
        textentry->scale.gravity = EASTWEST;
        textentry->parent = wid;
        Widget_t *okay = add_button(wid, "OK", 230, 20, 40, 30);
        okay->scale.gravity = CENTER;
        okay->parent = wid;
        okay->func.value_changed_callback = message_okay_callback;
        widget_show_all(wid);
        return wid;
    }

private:

/****************************************************************
            Okay button callback forward input to caller
            and destroy the window
****************************************************************/

    static void message_okay_callback(void *w_, void* user_data) {
        Widget_t *wid = (Widget_t*)w_;
        if (wid->flags & HAS_POINTER && !adj_get_value(wid->adj)){
            Widget_t *p = (Widget_t*)wid->parent;
            Widget_t *textentry = p->childlist->childs[0];
            Widget_t *pp = (Widget_t*)p->parent;
            if (strlen( textentry->input_label)) {
                textentry->input_label[strlen( textentry->input_label)-1] = 0;
                textentry->label = textentry->input_label;
                pp->func.dialog_callback(pp,&textentry->label);
            }
            destroy_widget(p, p->app);
        }
    }

/****************************************************************
                Text input handling
****************************************************************/

    static void entry_add_text(void  *w_, void *label_) {
        Widget_t *w = (Widget_t*)w_;
        if (!w) return;
        char *label = (char*)label_;
        if (!label) {
            label = (char*)"";
        }
        draw_entry(w,NULL);
        cairo_text_extents_t extents;
        use_text_color_scheme(w, NORMAL_);
        cairo_set_font_size (w->cr, 11.0);
        if (strlen( w->input_label))
             w->input_label[strlen( w->input_label)-1] = 0;
        if (strlen( w->input_label)<30) {
            if (strlen(label))
            strcat( w->input_label, label);
        }
        w->label = w->input_label;
        strcat( w->input_label, "|");
        cairo_set_font_size (w->cr, 12.0);
        cairo_text_extents(w->cr, w->input_label , &extents);

        cairo_move_to (w->cr, 2, 6.0+extents.height);
        cairo_show_text(w->cr,  w->input_label);

    }

    static void entry_clip(Widget_t *w) {
        draw_entry(w,NULL);
        cairo_text_extents_t extents;
        use_text_color_scheme(w, NORMAL_);
        cairo_set_font_size (w->cr, 11.0);

        // check for UTF 8 char
        if (strlen( w->input_label)>=2) {
            int i = strlen( w->input_label)-1;
            int j = 0;
            int u = 0;
            for(;i>0;i--) {
                if(IS_UTF8(w->input_label[i])) {
                     u++;
                }
                j++;
                if (u == 1) break;
                if (j>2) break;
            }
            if (!u) j =2;

            memset(&w->input_label[strlen( w->input_label)-(sizeof(char)*(j))],0,sizeof(char)*(j));
            strcat( w->input_label, "|");
        }
        cairo_set_font_size (w->cr, 12.0);
        cairo_text_extents(w->cr, w->input_label , &extents);

        cairo_move_to (w->cr, 2, 6.0+extents.height);
        cairo_show_text(w->cr,  w->input_label);

    }


    static void save_on_enter(Widget_t *wid) {
        Widget_t *p = (Widget_t*)wid->parent;
        Widget_t *textentry = p->childlist->childs[0];
        Widget_t *pp = (Widget_t*)p->parent;
        if (strlen( textentry->input_label)) {
            textentry->input_label[strlen( textentry->input_label)-1] = 0;
            textentry->label = textentry->input_label;
            pp->func.dialog_callback(pp,&textentry->label);
        }
        destroy_widget(p, p->app);
    }

    static void entry_get_text(void *w_, void *key_, void *user_data) {
        Widget_t *w = (Widget_t*)w_;
        if (!w) return;
        XKeyEvent *key = (XKeyEvent*)key_;
        if (!key) return;
        int nk = key_mapping(w->app->dpy, key);
        if (nk == 11) {
            entry_clip(w);
        } else {
            char buf[32];
            memset(buf, 0, 32);
            bool status = os_get_keyboard_input(w, key, buf, sizeof(buf) - 1);
            if (key_mapping(w->app->dpy, key) == 10) {
                save_on_enter(w);
                return;
            }
            if(status){
                entry_add_text(w, buf);
            }
        }
        os_expose_widget(w);
    }

/****************************************************************
                        Drawings
****************************************************************/

    static void setFrameColour(Widget_t* w, int x, int y, int wi, int h) {
        Colors *c = get_color_scheme(w, NORMAL_);
        cairo_pattern_t *pat = cairo_pattern_create_linear (x, y, x, y + h);
        cairo_pattern_add_color_stop_rgba
            (pat, 1, c->bg[0]*1.9, c->bg[1]*1.9, c->bg[2]*1.9,1.0);
        cairo_pattern_add_color_stop_rgba
            (pat, 0, c->bg[0]*0.2, c->bg[1]*0.2, c->bg[2]*0.2,1.0);
        cairo_set_source(w->cr, pat);
        cairo_pattern_destroy (pat);
    }

    static void draw_window(void *w_, void* user_data) {
        Widget_t *w = (Widget_t*)w_;
        use_bg_color_scheme(w, NORMAL_);
        cairo_paint(w->crb);
    }

    static void draw_entry(void *w_, void* user_data) {
        Widget_t *w = (Widget_t*)w_;
        if (!w) return;
        Metrics_t metrics;
        os_get_window_metrics(w, &metrics);
        int width = metrics.width-2;
        int height = metrics.height-2;
        if (!metrics.visible) return;

        use_base_color_scheme(w, NORMAL_);
        cairo_rectangle(w->cr,0,0,width,height);
        cairo_fill_preserve (w->cr);
        setFrameColour(w,0,0,width,height);
        cairo_set_line_width(w->cr, 2.0);
        cairo_stroke(w->cr);

        use_text_color_scheme(w, NORMAL_);
        cairo_set_font_size (w->cr, 9.0);

        cairo_move_to (w->cr, 2, 9);
        cairo_show_text(w->cr, " ");
    }

};

#endif
