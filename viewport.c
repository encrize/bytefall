#include "app.h"

const char *MODE_NAMES[] = { "8G", "RGB", "RGBA", "16G" };
const int   MODE_BPP[]   = { 1, 3, 4, 2 };

size_t row_stride(App *app)
{
    return (size_t)app->rnd.view_w_px * MODE_BPP[app->rnd.mode];
}

void decode_pixel(App *app, size_t byte_idx, Uint8 *r, Uint8 *g, Uint8 *b)
{
    Uint8 rv = 0, gv = 0, bv = 0;
    switch (app->rnd.mode) {
    case MODE_8G:
        if (byte_idx < app->pb.size) rv = gv = bv = app->pb.data[byte_idx];
        break;
    case MODE_RGB:
    case MODE_RGBA:
        if (byte_idx + 0 < app->pb.size) rv = app->pb.data[byte_idx + 0];
        if (byte_idx + 1 < app->pb.size) gv = app->pb.data[byte_idx + 1];
        if (byte_idx + 2 < app->pb.size) bv = app->pb.data[byte_idx + 2];
        break;
    case MODE_16G: {
        Uint16 v = 0;
        if (byte_idx + 0 < app->pb.size)
            v  = (Uint16)(app->pb.data[byte_idx + 0] << 8);
        if (byte_idx + 1 < app->pb.size)
            v |= (Uint16)(app->pb.data[byte_idx + 1]);
        rv = gv = bv = (Uint8)(v >> 8);
        break;
    }
    default: break;
    }
    *r = rv; *g = gv; *b = bv;
}


void render_waterfall(SDL_Renderer *ren, App *app, Viewport *vp)
{
    RenderState *rnd = &app->rnd;

    if (!rnd->wf_tex || rnd->wf_tex_w != vp->w || rnd->wf_tex_h != vp->h) {
        if (rnd->wf_tex) SDL_DestroyTexture(rnd->wf_tex);
        rnd->wf_tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_STREAMING, vp->w, vp->h);
        rnd->wf_tex_w = vp->w;
        rnd->wf_tex_h = vp->h;
    }

    void *pixels;
    int   pitch;
    SDL_LockTexture(rnd->wf_tex, NULL, &pixels, &pitch);
    Uint32 *px = (Uint32 *)pixels;

    for (int row = 0; row < vp->rows; row++) {
        int rfb = vp->rows - 1 - row;
        int yp  = row * PIXEL_H - rnd->scroll_y;
        if (yp + PIXEL_H <= 0 || yp >= vp->h) continue;

        float age = 1.0f - (float)rfb / (float)(vp->rows > 1 ? vp->rows - 1 : 1);
        long long bs = (long long)vp->base - (long long)rfb * (long long)vp->stride;

        if (bs < 0) {
            for (int py = yp; py < yp + PIXEL_H && py < vp->h; py++) {
                if (py < 0) continue;
                for (int x = 0; x < vp->w; x++)
                    px[py * (pitch / 4) + x] = 0xFF161414;
            }
            continue;
        }

        for (int x = 0; x < vp->w; x++) {
            int src_col = (int)((double)x * rnd->view_w_px / (double)vp->w);
            if (src_col >= rnd->view_w_px) src_col = rnd->view_w_px - 1;

            size_t bi = (size_t)bs + (size_t)src_col * MODE_BPP[rnd->mode];
            Uint8 r, g, b;
            decode_pixel(app, bi, &r, &g, &b);

            r = (Uint8)(r * age);
            g = (Uint8)(g * age);
            b = (Uint8)(b * age);

            Uint32 color = r | (g << 8) | (b << 16) | (0xFF << 24);
            for (int py = yp; py < yp + PIXEL_H && py < vp->h; py++) {
                if (py < 0) continue;
                px[py * (pitch / 4) + x] = color;
            }
        }
    }
    SDL_UnlockTexture(rnd->wf_tex);

    SDL_Rect dst = { vp->x, vp->y, vp->w, vp->h };
    SDL_RenderCopy(ren, rnd->wf_tex, NULL, &dst);

    set_col(ren, 0x28FFFFFF);
    SDL_RenderDrawLine(ren,
        vp->x, vp->y + vp->h - PIXEL_H,
        vp->x + vp->w, vp->y + vp->h - PIXEL_H);
}


void render_progress(SDL_Renderer *ren, App *app, Viewport *vp)
{
    if (app->pb.size == 0) return;
    int bx = vp->x, by = vp->y + vp->h, bw = vp->w, bh = 2;
    fill_rect(ren, bx, by, bw, bh, C_BTN);
    int filled = (int)((double)bw * (app->pb.pos / (double)app->pb.size));
    if (filled > bw) filled = bw;
    fill_rect(ren, bx, by, filled, bh, C_ACT);
}


void minimap_invalidate(App *app)
{
    app->rnd.mm_gen = -1;
}

void minimap_cleanup(App *app)
{
    if (app->rnd.mm_tex) {
        SDL_DestroyTexture(app->rnd.mm_tex);
        app->rnd.mm_tex = NULL;
    }
}

void render_minimap(SDL_Renderer *ren, App *app, Viewport *vp, int ww)
{
    int sx = ww - SCR_W, sy = vp->y, sh = vp->h;
    fill_rect(ren, sx, sy, SCR_W, sh, 0x1E1E20FF);

    if (app->pb.size == 0 || vp->stride == 0) return;

    RenderState *rnd = &app->rnd;
    bool cache_valid =
        (rnd->mm_tex != NULL) &&
        (rnd->mm_gen    == app->pb.file_gen) &&
        (rnd->mm_mode   == app->rnd.mode) &&
        (rnd->mm_w      == SCR_W) &&
        (rnd->mm_h      == sh);

    if (cache_valid) {
        SDL_Rect dst = { sx, sy, SCR_W, sh };
        SDL_RenderCopy(ren, rnd->mm_tex, NULL, &dst);
        return;
    }

    if (rnd->mm_tex) SDL_DestroyTexture(rnd->mm_tex);
    rnd->mm_tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32,
                    SDL_TEXTUREACCESS_STREAMING, SCR_W, sh);

    void *pixels;
    int   pitch;
    SDL_LockTexture(rnd->mm_tex, NULL, &pixels, &pitch);
    Uint32 *px = (Uint32 *)pixels;

    for (int y = 0; y < sh; y++) {
        double frac   = (double)y / (double)sh;
        size_t offset = (size_t)(frac * (double)app->pb.size);
        offset = (offset / vp->stride) * vp->stride;
        Uint8 r, g, b;
        decode_pixel(app, offset, &r, &g, &b);
        Uint32 color = r | (g << 8) | (b << 16) | (0xFF << 24);
        for (int x = 0; x < SCR_W; x++)
            px[y * (pitch / 4) + x] = color;
    }
    SDL_UnlockTexture(rnd->mm_tex);

    rnd->mm_gen  = app->pb.file_gen;
    rnd->mm_mode = app->rnd.mode;
    rnd->mm_w    = SCR_W;
    rnd->mm_h    = sh;

    SDL_Rect dst = { sx, sy, SCR_W, sh };
    SDL_RenderCopy(ren, rnd->mm_tex, NULL, &dst);
}


void render_scrollbar(SDL_Renderer *ren, App *app, Viewport *vp, int ww)
{
    if (app->pb.size == 0) return;
    int sx = ww - SCR_W, sy = vp->y, sh = vp->h;
    int thumb_h = sh / 8;
    if (thumb_h < 24) thumb_h = 24;
    if (thumb_h > sh) thumb_h = sh;

    double frac    = app->pb.pos / (double)app->pb.size;
    int    thumb_y = sy + (int)(frac * (sh - thumb_h));
    if (thumb_y < sy) thumb_y = sy;
    if (thumb_y + thumb_h > sy + sh) thumb_y = sy + sh - thumb_h;

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    bool on_thumb = (mx >= sx && mx < sx + SCR_W &&
                     my >= thumb_y && my < thumb_y + thumb_h);

    fill_rect(ren, sx + 1, thumb_y, SCR_W - 2, thumb_h,
              (app->ui.scr_drag || on_thumb) ? 0x6060AAFF : 0x44444AFF);
    draw_border(ren, sx + 1, thumb_y, SCR_W - 2, thumb_h, 0xAAAAAAFF);
}


void update_tooltip(App *app, Viewport *vp)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    if (mx < vp->w && my >= vp->y && my < vp->y + vp->h && app->pb.data) {
        int src_col = (int)((double)mx * app->rnd.view_w_px / (double)vp->w);
        if (src_col >= app->rnd.view_w_px) src_col = app->rnd.view_w_px - 1;
        int rfb = (vp->h - 1 - (my - vp->y) + app->rnd.scroll_y) / PIXEL_H;
        size_t byte_off = vp->base - (size_t)rfb * vp->stride
                        + (size_t)src_col * MODE_BPP[app->rnd.mode];

        if (byte_off < app->pb.size) {
            int  bpp = MODE_BPP[app->rnd.mode];
            char hex[32] = {0};
            for (int i = 0; i < bpp && byte_off + i < app->pb.size; i++)
                sprintf(hex + strlen(hex), "%02X ", app->pb.data[byte_off + i]);
            snprintf(app->ui.tooltip, sizeof(app->ui.tooltip),
                     "Off: 0x%08zX | %s", byte_off, hex);
            app->ui.tooltip_x = mx + 12;
            app->ui.tooltip_y = my - 8;
        } else {
            app->ui.tooltip[0] = 0;
        }
    } else {
        app->ui.tooltip[0] = 0;
    }
}