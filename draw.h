#ifndef DRAW_H
#define DRAW_H

struct SDL_Renderer;

void set_col(struct SDL_Renderer *r, Uint32 rgba);
void fill_rect(struct SDL_Renderer *r, int x, int y, int w, int h, Uint32 c);
void draw_border(struct SDL_Renderer *r, int x, int y, int w, int h, Uint32 c);

void draw_char(struct SDL_Renderer *r, int x, int y, char c, int sc, Uint32 col);
void draw_str(struct SDL_Renderer *r, int x, int y, const char *s, int sc, Uint32 col);
int  str_w(const char *s, int sc);

typedef struct { int x, y, w, h; } SimpleRect;
bool in_rect(int mx, int my, SimpleRect b);

#endif
