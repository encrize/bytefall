#ifndef RENDER_H
#define RENDER_H

struct App;
struct SDL_Renderer;

void update(struct App *app);
void render(struct SDL_Renderer *ren, struct App *app, int ww, int wh);

#endif
