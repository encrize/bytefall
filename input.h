#ifndef INPUT_H
#define INPUT_H

struct App;
union SDL_Event;

void handle_keyboard(struct App *app, union SDL_Event *ev);
void handle_mouse(struct App *app, union SDL_Event *ev, int ww, int wh);

#endif
