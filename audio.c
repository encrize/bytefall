#include "app.h"

static void audio_cb(void *ud, Uint8 *stream, int len)
{
    App *app = (App *)ud;
    Sint16 *out = (Sint16 *)stream;
    int n = len / 2;

    if (!app->pb.data || app->pb.size == 0 ||
        !SDL_AtomicGet(&app->pb.playing)) {
        SDL_memset(stream, 0, len);
        return;
    }

    float gain  = app->ui.muted ? 0.0f : (app->ui.vol_norm * MAX_GAIN);
    double speed = app->pb.speed;
    if (speed < 0.001) speed = 0.001;

    for (int i = 0; i < n; i++) {
        double exact_pos = (double)app->pb.audio_pos + app->pb.audio_accum;
        size_t p1   = (size_t)exact_pos;
        size_t p2   = p1 + 1;
        float  frac = (float)(exact_pos - (double)p1);

        Sint16 s1 = (p1 < app->pb.size)
            ? (Sint16)(((int)app->pb.data[p1] - 128) << 8) : 0;
        Sint16 s2 = (p2 < app->pb.size)
            ? (Sint16)(((int)app->pb.data[p2] - 128) << 8) : 0;
        Sint16 s  = (Sint16)(s1 + (s2 - s1) * frac);
        out[i] = (Sint16)(s * gain);

        app->pb.audio_accum += speed;
        int steps = (int)app->pb.audio_accum;
        if (steps > 0) {
            app->pb.audio_pos  += steps;
            app->pb.audio_accum -= steps;
        }
    }
}

void playback_seek(App *app, double pos)
{
    if (pos < 0) pos = 0;
    if (app->pb.size > 0 && pos > (double)app->pb.size)
        pos = (double)app->pb.size;

    app->pb.pos          = pos;
    app->pb.audio_pos    = (size_t)pos;
    app->pb.audio_accum  = 0.0;
}

void playback_restart(App *app)
{
    playback_seek(app, 0.0);
    SDL_AtomicSet(&app->pb.playing, 1);
}

void audio_init(App *app)
{
    SDL_AudioSpec want = {0}, got;
    want.freq     = AUDIO_RATE;
    want.format   = AUDIO_S16SYS;
    want.channels = 1;
    want.samples  = AUDIO_BUF;
    want.callback = audio_cb;
    want.userdata = app;

    app->pb.adev = SDL_OpenAudioDevice(NULL, 0, &want, &got, 0);
    if (app->pb.adev)
        SDL_PauseAudioDevice(app->pb.adev, 0);
}