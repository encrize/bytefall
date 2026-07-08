#include "app.h"


void do_flash(App *app, const char *msg, bool ok, int ms)
{
    snprintf(app->ui.flash, sizeof(app->ui.flash), "%s", msg);
    app->ui.flash_ok    = ok;
    app->ui.flash_until = SDL_GetTicks() + ms;
}


void draw_btn(SDL_Renderer *r, SimpleRect b, const char *lbl,
              bool hov, bool active)
{
    Uint32 bg  = active ? C_ACT_BG : (hov ? C_BTN_HOV : C_BTN);
    Uint32 brd = active ? C_ACT    : (hov ? 0x8888AAFF : C_BTN_BRD);
    Uint32 fg  = active ? C_ACT    : C_TEXT;

    fill_rect(r, b.x, b.y, b.w, b.h, bg);
    draw_border(r, b.x, b.y, b.w, b.h, brd);
    int tw = str_w(lbl, 2);
    draw_str(r, b.x + (b.w - tw) / 2, b.y + (b.h - 14) / 2, lbl, 2, fg);
}


void layout_all(App *app, int ww, int wh)
{
    UIState *ui = &app->ui;
    int rowA_y = wh - BOT_H + 8;
    int rowB_y = rowA_y + SB_H + 8;

    int x = 10;
    for (int m = 0; m < 4; m++) {
        int w = str_w(MODE_NAMES[m], 2) + 18;
        ui->btns[BID_8G + m] = (Btn){
            { x, rowA_y, w, SB_H }, MODE_NAMES[m], BID_8G + m
        };
        x += w + 6;
    }

    int vol_label_w = str_w("VOL", 2);
    int vol_label_x = x + 14;
    int slider_w = 90, slider_h = 10;
    ui->vol_slider.x = vol_label_x + vol_label_w + 8;
    ui->vol_slider.y = rowA_y + (SB_H - slider_h) / 2;
    ui->vol_slider.w = slider_w;
    ui->vol_slider.h = slider_h;

    int help_w = str_w("?", 2) + 18;
    int mute_w = str_w("MUTE", 2) + 18;
    int rx = ww - 10;
    rx -= help_w;
    ui->btns[BID_HELP] = (Btn){ { rx, rowA_y, help_w, SB_H }, "?", BID_HELP };
    rx -= 8;
    rx -= mute_w;
    ui->btns[BID_MUTE] = (Btn){ { rx, rowA_y, mute_w, SB_H }, "MUTE", BID_MUTE };

    int rw_w   = str_w("<<", 2) + 18;
    int ff_w   = str_w(">>", 2) + 18;
    int play_w = str_w("PAUSE", 2) + 22;
    int total  = rw_w + 8 + play_w + 8 + ff_w;
    int cx = (ww - total) / 2;
    if (cx < 10) cx = 10;

    ui->btns[BID_RW]   = (Btn){ { cx, rowB_y, rw_w,   SB_H }, "<<",    BID_RW };   cx += rw_w + 8;
    ui->btns[BID_PLAY] = (Btn){ { cx, rowB_y, play_w, SB_H }, "PLAY",  BID_PLAY }; cx += play_w + 8;
    ui->btns[BID_FF]   = (Btn){ { cx, rowB_y, ff_w,   SB_H }, ">>",    BID_FF };

    ui->speed_pos.x = ui->btns[BID_FF].r.x + ui->btns[BID_FF].r.w + 14;
    ui->speed_pos.y = rowB_y;
}


void draw_volume(SDL_Renderer *ren, App *app, int rowA_y)
{
    UIState *ui = &app->ui;

    int label_x = ui->btns[BID_16G].r.x + ui->btns[BID_16G].r.w + 14;
    Uint32 lc = ui->muted ? C_MUTED : C_TEXT;
    draw_str(ren, label_x, rowA_y + (SB_H - 14) / 2, "VOL", 2, lc);

    fill_rect(ren, ui->vol_slider.x, ui->vol_slider.y,
              ui->vol_slider.w, ui->vol_slider.h, C_BTN);
    draw_border(ren, ui->vol_slider.x - 1, ui->vol_slider.y - 1,
                ui->vol_slider.w + 2, ui->vol_slider.h + 2, C_BTN_BRD);

    int filled = (int)(ui->vol_norm * ui->vol_slider.w);
    fill_rect(ren, ui->vol_slider.x, ui->vol_slider.y, filled,
              ui->vol_slider.h, ui->muted ? C_MUTED : C_ACT);

    int knob_x = ui->vol_slider.x + filled - 3;
    fill_rect(ren, knob_x, ui->vol_slider.y - 3, 6,
              ui->vol_slider.h + 6, ui->muted ? C_MUTED : C_TEXT);
}


void draw_help(SDL_Renderer *r, int ww, int wh)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0x14, 0x14, 0x16, 0xD0);
    SDL_Rect full = { 0, 0, ww, wh };
    SDL_RenderFillRect(r, &full);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    int pw = 380, ph = 300, px = (ww - pw) / 2, py = (wh - ph) / 2;
    fill_rect(r, px, py, pw, ph, C_BAR);
    draw_border(r, px, py, pw, ph, C_DIV);
    draw_border(r, px + 1, py + 1, pw - 2, ph - 2, 0x444448FF);

    const char *title = "Controls & Shortcuts";
    draw_str(r, px + (pw - str_w(title, 2)) / 2, py + 14, title, 2, C_TEXT);
    fill_rect(r, px + 12, py + 34, pw - 24, 1, C_DIV);

    static const char *keys[][2] = {
        { "Space",       "Play / Pause"        },
        { "R",           "Restart"             },
        { "M",           "Mute / Unmute"       },
        { "Up / Down",   "Speed x2 / x0.5"     },
        { "Wheel",       "Scroll / Seek"       },
        { "Ctrl+Wheel",  "Zoom In / Out"       },
        { "Scroll Drag", "Click track to jump" },
        { "Drag & Drop", "Load any file"       },
    };
    int ny = py + 46;
    for (int i = 0; i < 8; i++) {
        draw_str(r, px + 18,  ny, keys[i][0], 1, C_ACT);
        draw_str(r, px + 160, ny, keys[i][1], 1, C_MUTED);
        ny += 22;
    }

    const char *cl = "[?] or [Esc] to close";
    draw_str(r, px + (pw - str_w(cl, 1)) / 2, py + ph - 18, cl, 1, C_MUTED);
}