
/*
 * SizeGroup.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2026 brummer <brummer@web.de>
 */

/****************************************************************
        SizeGroup.h - a vertical animated size group 
                      with drag and drop support for libxputty
****************************************************************/


#pragma once
#include "xwidgets.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    Widget_t* w;
    int x0,y0,x1,y1;
    float t;
} VSTween;

typedef struct {

    Widget_t* parent;
    Widget_t* dragWidget;

    Widget_t** entries;
    int entryCount;
    int entryCap;

    VSTween* tweens;
    int tweenCount;
    int tweenCap;

    int startX,startY;
    int spacingY;

    int dragOffsetY;
    int wmy;

    int from,to;
    int oldIndex,newIndex;

    int* glowY;

    int animateOnAdd;
    int tweensActive;

} VerticalSizeGroup;

static inline void vsg_destroy(VerticalSizeGroup* g) {
    if (!g) return;

    if (g->entries) {
        free(g->entries);
        g->entries = NULL;
    }

    if (g->tweens) {
        free(g->tweens);
        g->tweens = NULL;
    }

    g->entryCount = 0;
    g->entryCap   = 0;
    g->tweenCount = 0;
    g->tweenCap   = 0;

    g->parent     = NULL;
    g->dragWidget = NULL;
}

static inline void vsg_init(VerticalSizeGroup* g,
    Widget_t* parent,int sx,int sy,int spacingY,int* glowY) {
    memset(g,0,sizeof(*g));     // clear the SizeGroup
    g->parent = parent;         // the parent window
    g->startX = sx;             // the upper left corner
    g->startY = sy;             // the upper vertical point
    g->spacingY = spacingY;     // spacing behind elements
    g->glowY = glowY;           // drop indicator
    g->animateOnAdd = 1;        // switch animation on/off
    g->newIndex = 1;            // the drop index 
    g->wmy = sy;                // the current position from a dragged window 
}

// call from a GUI timeout loop (60fps)
static inline void vsg_update(VerticalSizeGroup* g,float dt) {
    if (!g->tweensActive) return;
    Display* dpy = g->parent->app->dpy;
    int active = 0;

    for (int i=0;i<g->tweenCount;i++) {
        VSTween* t = &g->tweens[i];
        if (t->t>=1) continue;

        t->t += dt/0.083335f; // 60fps
        if (t->t>1) t->t=1;

        float s = t->t*t->t*(3-2*t->t);
        int y = t->y0 + (t->y1 - t->y0)*s;
        os_move_window(dpy,t->w,t->x0,y);
        if (t->t<1) active=1;
    }
    g->tweensActive = active;
}

// prepare entries for animation/ positioning 
static inline void vsg_relayout(VerticalSizeGroup* g) {
    Display* dpy = g->parent->app->dpy;
    g->tweenCount = 0;
    int y = g->startY;

    for (int i=0;i<g->entryCount;i++) {
        Widget_t* w = g->entries[i];

        int ty = y;
        int fy = w->scale.init_y;

        if (g->animateOnAdd && i>=g->from && i<g->to) {
            if (g->from==0 && g->to==g->entryCount) {
                fy = g->startY - 80;
            } else if (i==g->newIndex) {
                fy = g->wmy;
            }

            if (g->tweenCount+1 > g->tweenCap) {
                g->tweenCap = g->tweenCap ? g->tweenCap*2 : 8;
                g->tweens = (VSTween*)realloc(g->tweens,sizeof(VSTween)*g->tweenCap);
            }

            g->tweens[g->tweenCount++] = (VSTween){w,w->scale.init_x,fy,w->scale.init_x,ty,0};
            os_move_window(dpy,w,w->scale.init_x,fy);
            g->tweensActive = 1;
        } else {
            os_move_window(dpy,w,w->scale.init_x,ty);
        }

        w->scale.init_y = ty;
        y += w->height + g->spacingY;
    }
}

// add element to the size-group 
static inline void vsg_add(VerticalSizeGroup* g,Widget_t* w) {
    if (g->entryCount+1 > g->entryCap) {
        g->entryCap = g->entryCap ? g->entryCap*2 : 8;
        g->entries = (Widget_t**)realloc(g->entries,sizeof(void*)*g->entryCap);
    }
    g->entries[g->entryCount++] = w;
    g->to = g->entryCount;
    vsg_relayout(g);
}

// find index of a element in the size-group
static inline int vsg_findDragIndex(VerticalSizeGroup* g, Widget_t* w) {
    for (int i=0;i<g->entryCount;i++) {
        if (g->entries[i] == w) {
            g->oldIndex = i;
            return i;
        }
    }
    return -1;
}

// find new index of a moved element
static inline int vsg_findDropIndex(VerticalSizeGroup* g) {
    int best = 0;
    int bestDist = 1e9;

    for (int i=0;i<g->entryCount;i++) {
        Widget_t* w = g->entries[i];

        int cy = w->scale.init_y;
        int dy = abs(g->wmy - cy);
        if (dy < bestDist) {
            best = i;
            bestDist = dy;

            if (i > g->oldIndex)
                cy += w->height + g->spacingY;
            *g->glowY = cy - g->spacingY/2;
        }
    }
    return best;
}

// register a element for dragging
static inline void vsg_beginDrag(VerticalSizeGroup* g,Widget_t* w,int my) {
    g->dragWidget = w;
    g->dragOffsetY = my;
    os_raise_widget(w);
}

// while move the element
static inline void vsg_dragMove(VerticalSizeGroup* g,int my) {
    if (!g->dragWidget) return;

    g->wmy = g->dragWidget->scale.init_y + my - g->dragOffsetY;
    os_move_window(g->parent->app->dpy,
                   g->dragWidget,
                   g->dragWidget->scale.init_x,
                   g->wmy);

    // find current index
    for (int i=0;i<g->entryCount;i++) {
        if (g->entries[i] == g->dragWidget) {
            g->oldIndex = i;
            break;
        }
    }

    g->newIndex = vsg_findDropIndex(g);

    expose_widget(g->parent);
}

// insert dropped element in size-group at new index
static inline void vsg_endDrag(VerticalSizeGroup* g) {
    if (!g->dragWidget) return;

    if (g->newIndex != g->oldIndex) {
        Widget_t* w = g->dragWidget;
        memmove(&g->entries[g->oldIndex],
                &g->entries[g->oldIndex+1],
                sizeof(void*)*(g->entryCount-g->oldIndex-1));
        memmove(&g->entries[g->newIndex+1],
                &g->entries[g->newIndex],
                sizeof(void*)*(g->entryCount-g->newIndex-1));
        g->entries[g->newIndex] = w;
    }

    g->from = g->oldIndex<g->newIndex?g->oldIndex:g->newIndex;
    g->to   = g->oldIndex>g->newIndex?g->oldIndex+1:g->newIndex+1;

    vsg_relayout(g);
    g->dragWidget = NULL;
    *g->glowY = -1;
    expose_widget(g->parent);
}
