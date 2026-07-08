#ifndef AUDIO_H
#define AUDIO_H

struct App;

void audio_init(struct App *app);

void playback_seek(struct App *app, double pos);

void playback_restart(struct App *app);

#endif
