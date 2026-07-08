#include "app.h"

void load_file(App *app, const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) { do_flash(app, "Cannot open file", false, 3000); return; }

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz < 0) { fclose(fp); do_flash(app, "Cannot read file", false, 3000); return; }

    Uint8 *nd = (Uint8 *)malloc(sz ? (size_t)sz : 1);
    if (!nd) { fclose(fp); do_flash(app, "Out of memory", false, 3000); return; }

    if (sz > 0 && fread(nd, 1, (size_t)sz, fp) != (size_t)sz) {
        free(nd); fclose(fp);
        do_flash(app, "Read error", false, 3000); return;
    }
    fclose(fp);

    if (app->pb.data) free(app->pb.data);
    app->pb.data = nd;
    app->pb.size = (size_t)sz;

    const char *b = path;
    const char *sl = strrchr(path, '/');
    const char *bs = strrchr(path, '\\');
    if (sl && sl >= b) b = sl + 1;
    if (bs && bs >= b) b = bs + 1;
    snprintf(app->pb.name, sizeof(app->pb.name), "%s", b);

    app->pb.file_gen++;
    playback_seek(app, 0.0);
    SDL_AtomicSet(&app->pb.playing, 1);
    app->rnd.scroll_y = 0;
    minimap_invalidate(app);

    char buf[64];
    if (app->pb.size >= (size_t)(1 << 20))
        snprintf(buf, sizeof(buf), "Loaded %.1f MB", (double)app->pb.size / (1 << 20));
    else if (app->pb.size >= 1024)
        snprintf(buf, sizeof(buf), "Loaded %.1f KB", (double)app->pb.size / 1024);
    else
        snprintf(buf, sizeof(buf), "Loaded %zu B", app->pb.size);
    do_flash(app, buf, true, 2500);
}