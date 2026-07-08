#include "app.h"


void set_col(SDL_Renderer *r, Uint32 rgba)
{
    SDL_SetRenderDrawColor(r,
        (rgba >> 24) & 0xff, (rgba >> 16) & 0xff,
        (rgba >>  8) & 0xff,  rgba        & 0xff);
}

void fill_rect(SDL_Renderer *r, int x, int y, int w, int h, Uint32 c)
{
    set_col(r, c);
    SDL_Rect rc = { x, y, w, h };
    SDL_RenderFillRect(r, &rc);
}

void draw_border(SDL_Renderer *r, int x, int y, int w, int h, Uint32 c)
{
    set_col(r, c);
    SDL_Rect rc = { x, y, w, h };
    SDL_RenderDrawRect(r, &rc);
}


void draw_char(SDL_Renderer *r, int x, int y, char c, int sc, Uint32 col)
{
    int i = (unsigned char)c - 32;
    if (i < 0 || i >= (int)(sizeof(F57) / 5)) return;
    set_col(r, col);
    for (int cx = 0; cx < 5; cx++) {
        Uint8 b = F57[i][cx];
        for (int ry = 0; ry < 7; ry++) {
            if (b & (1 << ry)) {
                SDL_Rect rc = { x + cx * sc, y + ry * sc, sc, sc };
                SDL_RenderFillRect(r, &rc);
            }
        }
    }
}

void draw_str(SDL_Renderer *r, int x, int y, const char *s, int sc, Uint32 col)
{
    while (*s) {
        draw_char(r, x, y, *s, sc, col);
        x += 6 * sc;
        s++;
    }
}

int str_w(const char *s, int sc)
{
    return (int)strlen(s) * 6 * sc;
}


bool in_rect(int mx, int my, SimpleRect b)
{
    return mx >= b.x && mx < b.x + b.w &&
           my >= b.y && my < b.y + b.h;
}