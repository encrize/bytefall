#ifndef VIEWPORT_H
#define VIEWPORT_H

struct App;
struct SDL_Renderer;

typedef struct {
    int    x, y, w, h;
    size_t base, stride;
    int    rows;
} Viewport;

size_t row_stride(struct App *app);
void   decode_pixel(struct App *app, size_t byte_idx,
                    Uint8 *r, Uint8 *g, Uint8 *b);

void render_waterfall(struct SDL_Renderer *ren, struct App *app, Viewport *vp);
void render_progress(struct SDL_Renderer *ren, struct App *app, Viewport *vp);

void render_minimap(struct SDL_Renderer *ren, struct App *app,
                    Viewport *vp, int ww);
void minimap_invalidate(struct App *app);
void minimap_cleanup(struct App *app);

void render_scrollbar(struct SDL_Renderer *ren, struct App *app,
                      Viewport *vp, int ww);

void update_tooltip(struct App *app, Viewport *vp);

#endif
