#include "app.h"

int main(int argc, char **argv)
{
    App app = {0};
    app.rnd.view_w_px = 128;
    app.ui.vol_norm   = 0.4f;
    app.pb.speed      = 1.0;
    app.rnd.mm_gen    = -1;
    SDL_AtomicSet(&app.pb.playing, 0);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_Window *win = SDL_CreateWindow(
        "Bytefall",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_SetWindowMinimumSize(win, MIN_W, MIN_H);

    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    audio_init(&app);
    if (argc >= 2)
        load_file(&app, argv[1]);

    int        running = 1;
    SDL_Event  ev;

    while (running) {
        int ww, wh;
        SDL_GetWindowSize(win, &ww, &wh);
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        layout_all(&app, ww, wh);
        for (int i = 0; i < BID_COUNT; i++)
            app.ui.hov[i] = in_rect(mx, my, app.ui.btns[i].r);

        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT)
                running = 0;
            else if (ev.type == SDL_DROPFILE) {
                load_file(&app, ev.drop.file);
                SDL_free(ev.drop.file);
            }
            else if (ev.type == SDL_KEYDOWN)
                handle_keyboard(&app, &ev);
            else
                handle_mouse(&app, &ev, ww, wh);
        }

        update(&app);
        render(ren, &app, ww, wh);
        SDL_Delay(8);
    }

    if (app.pb.adev)   SDL_CloseAudioDevice(app.pb.adev);
    if (app.pb.data)   free(app.pb.data);
    if (app.rnd.wf_tex) SDL_DestroyTexture(app.rnd.wf_tex);
    minimap_cleanup(&app);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}