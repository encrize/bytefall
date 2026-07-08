#ifndef UI_H
#define UI_H

struct App;
struct SDL_Renderer;

void do_flash(struct App *app, const char *msg, bool ok, int ms);

void layout_all(struct App *app, int ww, int wh);

typedef struct { SimpleRect r; const char *lbl; int id; } Btn;
void draw_btn(struct SDL_Renderer *r, SimpleRect b, const char *lbl,
              bool hov, bool active);

void draw_volume(struct SDL_Renderer *ren, struct App *app, int rowA_y);

void draw_help(struct SDL_Renderer *r, int ww, int wh);

#endif
