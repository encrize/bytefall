#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "config.h"

#include "draw.h"
#include "audio.h"
#include "viewport.h"
#include "ui.h"
#include "input.h"
#include "render.h"
#include "fileio.h"


typedef struct {
    Uint8           *data;
    size_t           size;
    char             name[256];
    SDL_atomic_t     playing;
    double           pos;
    double           speed;
    SDL_AudioDeviceID adev;

    volatile size_t  audio_pos;
    double           audio_accum;

    int              file_gen;
} PlaybackState;

typedef struct {
    int      show_help;

    char     flash[160];
    Uint32   flash_until;
    int      flash_ok;

    SimpleRect vol_slider;
    int      vol_drag;
    float    vol_norm;
    int      muted;

    char     tooltip[128];
    int      tooltip_x, tooltip_y;

    int      scr_drag, scr_drag_y;
    double   scr_drag_pos;

    Btn      btns[BID_COUNT];
    int      hov[BID_COUNT];

    SimpleRect speed_pos;
} UIState;

typedef struct {
    PixelMode     mode;
    int           view_w_px;

    SDL_Texture  *wf_tex;
    int           wf_tex_w, wf_tex_h;
    int           scroll_y;

    SDL_Texture  *mm_tex;
    int           mm_w, mm_h;
    int           mm_gen;
    PixelMode     mm_mode;
} RenderState;

typedef struct App {
    PlaybackState pb;
    UIState       ui;
    RenderState   rnd;
} App;

#endif
