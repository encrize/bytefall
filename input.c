#include "app.h"



void handle_keyboard(App *app, SDL_Event *ev)
{
    if (app->ui.show_help) {
        if (ev->key.keysym.sym == SDLK_ESCAPE  ||
            ev->key.keysym.sym == SDLK_SLASH   ||
            ev->key.keysym.sym == SDLK_QUESTION ||
            ev->key.keysym.sym == SDLK_h)
            app->ui.show_help = 0;
        return;
    }

    switch (ev->key.keysym.sym) {
    case SDLK_ESCAPE:
    case SDLK_q:
        ev->type = SDL_QUIT;
        break;
    case SDLK_SPACE:
        SDL_AtomicSet(&app->pb.playing, !SDL_AtomicGet(&app->pb.playing));
        break;
    case SDLK_r:
        playback_restart(app);
        break;
    case SDLK_m:
        app->ui.muted = !app->ui.muted;
        break;
    case SDLK_h:
    case SDLK_SLASH:
        app->ui.show_help = !app->ui.show_help;
        break;
    case SDLK_UP:
        app->pb.speed *= 2.0;
        if (app->pb.speed > 32.0) app->pb.speed = 32.0;
        break;
    case SDLK_DOWN:
        app->pb.speed *= 0.5;
        if (app->pb.speed < 0.125) app->pb.speed = 0.125;
        break;
    default: break;
    }
}


static void mouse_down_volume(App *app, int bx, int by)
{
    if (in_rect(bx, by, *(SimpleRect *)&app->ui.vol_slider)) {
        app->ui.vol_drag = 1;
        float v = (float)(bx - app->ui.vol_slider.x) /
                  (float)app->ui.vol_slider.w;
        if (v < 0) v = 0;
        if (v > 1) v = 1;
        app->ui.vol_norm = v;
    }
}

static void mouse_down_scrollbar(App *app, int by, int wh)
{
    int vy = BAR_H, vh = wh - BAR_H - BOT_H;
    int sh = vh;
    if (app->pb.size == 0 || sh == 0) return;

    int thumb_h = sh / 8;
    if (thumb_h < 24) thumb_h = 24;

    int thumb_y = vy + (int)((app->pb.pos / (double)app->pb.size) * (sh - thumb_h));

    if (by >= thumb_y && by < thumb_y + thumb_h) {
        app->ui.scr_drag    = 1;
        app->ui.scr_drag_y  = by;
        app->ui.scr_drag_pos = app->pb.pos;
    } else {
        double nf = (double)(by - vy) / (double)sh;
        if (nf < 0) nf = 0;
        if (nf > 1) nf = 1;
        playback_seek(app, nf * (double)app->pb.size);
        app->ui.scr_drag    = 1;
        app->ui.scr_drag_y  = by;
        app->ui.scr_drag_pos = app->pb.pos;
    }
}

static void mouse_down_buttons(App *app, int bx, int by)
{
    (void)bx; (void)by;

    if (app->ui.hov[BID_PLAY])
        SDL_AtomicSet(&app->pb.playing, !SDL_AtomicGet(&app->pb.playing));

    if (app->ui.hov[BID_RW]) {
        size_t s = row_stride(app);
        playback_seek(app, app->pb.pos - (double)s * 30);
    }

    if (app->ui.hov[BID_FF] && app->pb.size) {
        size_t s = row_stride(app);
        playback_seek(app, app->pb.pos + (double)s * 30);
    }

    if (app->ui.hov[BID_MUTE])
        app->ui.muted = !app->ui.muted;

    if (app->ui.hov[BID_HELP])
        app->ui.show_help = !app->ui.show_help;

    for (int m = 0; m < 4; m++)
        if (app->ui.hov[BID_8G + m])
            app->rnd.mode = (PixelMode)m;
}

static void mouse_up(App *app)
{
    app->ui.scr_drag = 0;
    app->ui.vol_drag = 0;
}

static void mouse_motion(App *app, int mx, int my, int wh)
{
    if (app->ui.vol_drag) {
        float v = (float)(mx - app->ui.vol_slider.x) /
                  (float)app->ui.vol_slider.w;
        if (v < 0) v = 0;
        if (v > 1) v = 1;
        app->ui.vol_norm = v;
    }

    if (app->ui.scr_drag && app->pb.size > 0) {
        int sh = wh - BAR_H - BOT_H;
        int thumb_h = sh / 8;
        if (thumb_h < 24) thumb_h = 24;
        if (sh - thumb_h > 0) {
            double delta = (double)(my - app->ui.scr_drag_y) /
                           (double)(sh - thumb_h);
            playback_seek(app, app->ui.scr_drag_pos + delta * (double)app->pb.size);
        }
    }
}

static void mouse_wheel(App *app, SDL_Event *ev)
{
    if (app->ui.show_help) return;

    if (SDL_GetModState() & KMOD_CTRL) {
        app->rnd.view_w_px -= ev->wheel.y * 16;
        if (app->rnd.view_w_px < 32)   app->rnd.view_w_px = 32;
        if (app->rnd.view_w_px > 2048) app->rnd.view_w_px = 2048;
    } else {
        size_t s = row_stride(app);
        playback_seek(app, app->pb.pos - (double)ev->wheel.y * s * 5);
    }
}


void handle_mouse(App *app, SDL_Event *ev, int ww, int wh)
{
    switch (ev->type) {

    case SDL_MOUSEBUTTONDOWN:
        if (ev->button.button != SDL_BUTTON_LEFT) break;
    {
        int bx = ev->button.x, by = ev->button.y;

        if (app->ui.show_help) { app->ui.show_help = 0; return; }

        mouse_down_volume(app, bx, by);
        if (app->ui.vol_drag) return;

        int sx = ww - SCR_W;
        if (bx >= sx) {
            mouse_down_scrollbar(app, by, wh);
            return;
        }

        mouse_down_buttons(app, bx, by);
        break;
    }

    case SDL_MOUSEBUTTONUP:
        if (ev->button.button == SDL_BUTTON_LEFT)
            mouse_up(app);
        break;

    case SDL_MOUSEMOTION:
        mouse_motion(app, ev->motion.x, ev->motion.y, wh);
        break;

    case SDL_MOUSEWHEEL:
        mouse_wheel(app, ev);
        break;

    default: break;
    }
}