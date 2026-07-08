#include "app.h"



void update(App *app)
{
    if (SDL_AtomicGet(&app->pb.playing) &&
        app->pb.data && app->pb.size > 0 && !app->ui.show_help)
    {
        app->pb.pos = (double)app->pb.audio_pos;
        if (app->pb.audio_pos >= app->pb.size) {
            app->pb.pos = (double)app->pb.size;
            SDL_AtomicSet(&app->pb.playing, 0);
            do_flash(app, "End of file", true, 2000);
        }
    }
}


static void render_topbar(SDL_Renderer *ren, App *app, int ww)
{
    fill_rect(ren, 0, 0, ww, BAR_H, C_BAR);
    fill_rect(ren, 0, BAR_H - 1, ww, 1, C_DIV);

    if (!app->pb.name[0]) return;

    int tw = str_w(app->pb.name, 2);
    int tx = (ww - tw) / 2;
    if (tx < 8) tx = 8;
    draw_str(ren, tx, BAR_H / 2 - 7, app->pb.name, 2, C_TEXT);

    char sz[32];
    if (app->pb.size >= (size_t)(1 << 20))
        snprintf(sz, sizeof(sz), "%.1f MB", (double)app->pb.size / (1 << 20));
    else if (app->pb.size >= 1024)
        snprintf(sz, sizeof(sz), "%.1f KB", (double)app->pb.size / 1024);
    else
        snprintf(sz, sizeof(sz), "%zu B", app->pb.size);
    draw_str(ren, ww - str_w(sz, 1) - 10, BAR_H / 2 - 3, sz, 1, C_MUTED);

    char zbuf[32];
    snprintf(zbuf, sizeof(zbuf), "W:%d", app->rnd.view_w_px);
    draw_str(ren, 8, BAR_H / 2 - 3, zbuf, 1, C_MUTED);
}

static void render_waterfall_area(SDL_Renderer *ren, App *app, int ww, int wh)
{
    int vx = 0, vy = BAR_H;
    int vw = ww - SCR_W, vh = wh - BAR_H - BOT_H;
    if (vh <= 0 || vw <= 0) return;

    size_t stride = row_stride(app);
    if (stride > 0) {
        double rowf = app->pb.pos / (double)stride;
        double frac = rowf - floor(rowf);
        app->rnd.scroll_y = (int)(frac * (double)PIXEL_H);
    } else {
        app->rnd.scroll_y = 0;
    }

    int rows = vh / PIXEL_H + 2;
    size_t base = (size_t)app->pb.pos;
    base = (stride > 0) ? (base / stride) * stride : 0;

    Viewport vp = { vx, vy, vw, vh, base, stride, rows };

    render_waterfall(ren, app, &vp);
    render_progress(ren, app, &vp);
    render_minimap(ren, app, &vp, ww);
    render_scrollbar(ren, app, &vp, ww);
    update_tooltip(app, &vp);
}

static void render_empty_prompt(SDL_Renderer *ren, int ww, int wh)
{
    const char *m = "Drop any file onto the window";
    draw_str(ren, (ww - str_w(m, 1)) / 2, wh / 2 - 3, m, 1, C_MUTED);
}

static void render_bottom_bar(SDL_Renderer *ren, App *app, int ww, int wh)
{
    int rowA_y = wh - BOT_H + 8;

    fill_rect(ren, 0, wh - BOT_H, ww, BOT_H, C_BAR);
    fill_rect(ren, 0, wh - BOT_H, ww, 1, C_DIV);

    for (int m = 0; m < 4; m++)
        draw_btn(ren, app->ui.btns[BID_8G + m].r,
                 MODE_NAMES[m], app->ui.hov[BID_8G + m],
                 (int)app->rnd.mode == m);

    draw_volume(ren, app, rowA_y);

    draw_btn(ren, app->ui.btns[BID_MUTE].r, "MUTE",
             app->ui.hov[BID_MUTE], app->ui.muted);
    draw_btn(ren, app->ui.btns[BID_HELP].r, "?",
             app->ui.hov[BID_HELP], app->ui.show_help);

    draw_btn(ren, app->ui.btns[BID_RW].r, "<<",
             app->ui.hov[BID_RW], false);

    bool playing = SDL_AtomicGet(&app->pb.playing);
    draw_btn(ren, app->ui.btns[BID_PLAY].r,
             playing ? "PAUSE" : "PLAY",
             app->ui.hov[BID_PLAY], playing);

    draw_btn(ren, app->ui.btns[BID_FF].r, ">>",
             app->ui.hov[BID_FF], false);

    char spbuf[16];
    snprintf(spbuf, sizeof(spbuf), "x%.3g", app->pb.speed);
    draw_str(ren, app->ui.speed_pos.x,
             app->ui.speed_pos.y + (SB_H - 7) / 2,
             spbuf, 1, C_MUTED);
}

static void render_flash_notification(SDL_Renderer *ren, App *app, int ww, int wh)
{
    if (SDL_GetTicks() >= app->ui.flash_until) return;

    Uint32 bc = app->ui.flash_ok ? C_GREEN_BG : C_RED_BG;
    Uint32 tc = app->ui.flash_ok ? C_GREEN     : C_RED;
    int fw = str_w(app->ui.flash, 1) + 24, fh = 22;
    int fx = ww - fw - 10, fy = wh - BOT_H - fh - 8;

    fill_rect(ren, fx, fy, fw, fh, bc);
    draw_border(ren, fx, fy, fw, fh, tc);
    draw_str(ren, fx + 12, fy + (fh - 7) / 2, app->ui.flash, 1, tc);
}

static void render_tooltip_overlay(SDL_Renderer *ren, App *app, int ww)
{
    if (!app->ui.tooltip[0]) return;

    int tw = str_w(app->ui.tooltip, 1) + 12;
    int th = 16;
    int tx = app->ui.tooltip_x;
    int ty = app->ui.tooltip_y - th;

    if (tx + tw > ww - 5) tx = ww - tw - 5;
    if (ty < BAR_H + 2)   ty = app->ui.tooltip_y + 12;

    fill_rect(ren, tx, ty, tw, th, 0x18181AFF);
    draw_border(ren, tx, ty, tw, th, 0x44444AFF);
    draw_str(ren, tx + 6, ty + 4, app->ui.tooltip, 1, C_ACT);
}


void render(SDL_Renderer *ren, App *app, int ww, int wh)
{
    set_col(ren, C_BG);
    SDL_RenderClear(ren);

    render_topbar(ren, app, ww);
    render_waterfall_area(ren, app, ww, wh);

    if (!app->pb.data)
        render_empty_prompt(ren, ww, wh);

    render_bottom_bar(ren, app, ww, wh);
    render_flash_notification(ren, app, ww, wh);
    render_tooltip_overlay(ren, app, ww);

    if (app->ui.show_help)
        draw_help(ren, ww, wh);

    SDL_RenderPresent(ren);
}