#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define WIN_W   530
#define WIN_H   760
#define MIN_W   530
#define MIN_H   760
#define BAR_H    48
#define BOT_H    78
#define SCR_W    14
#define SB_H     30
#define PIXEL_H  3 

#define C_BG        0x141416FF
#define C_BAR       0x222224FF
#define C_DIV       0x333336FF
#define C_TEXT      0xF2F2F4FF
#define C_MUTED     0x8A8A90FF
#define C_BTN       0x38383CFF
#define C_BTN_HOV   0x505056FF
#define C_BTN_BRD   0x555558FF
#define C_ACT_BG    0x061830FF
#define C_ACT       0x0A84FFFF
#define C_GREEN     0x32D74BFF
#define C_GREEN_BG  0x0C2216FF
#define C_RED       0xFF3B30FF
#define C_RED_BG    0x2A0C0CFF

typedef enum { MODE_8G=0, MODE_RGB=1, MODE_RGBA=2, MODE_16G=3, MODE_COUNT } PixelMode;
static const char *MODE_NAMES[] = {"8G","RGB","RGBA","16G"};
static const int   MODE_BPP[]   = {1,3,4,2};

#define AUDIO_RATE  11025
#define AUDIO_BUF   512
#define MAX_GAIN    0.22f

static const Uint8 F57[][5]={
{0,0,0,0,0},{0,0,95,0,0},{0,7,0,7,0},{20,127,20,127,20},
{36,42,127,42,18},{35,19,8,100,98},{54,73,85,34,80},{0,5,3,0,0},
{0,28,34,65,0},{0,65,34,28,0},{20,8,62,8,20},{8,8,62,8,8},
{0,80,48,0,0},{8,8,8,8,8},{0,96,96,0,0},{32,16,8,4,2},
{62,81,73,69,62},{0,66,127,64,0},{66,97,81,73,70},{33,65,69,75,49},
{24,20,18,127,16},{39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},
{54,73,73,73,54},{6,73,73,41,30},{0,54,54,0,0},{0,86,54,0,0},
{8,20,34,65,0},{20,20,20,20,20},{0,65,34,20,8},{2,1,81,9,6},
{50,73,121,65,62},{126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},
{127,65,65,34,28},{127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},
{127,8,8,8,127},{0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},
{127,64,64,64,64},{127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},
{127,9,9,9,6},{62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},
{1,1,127,1,1},{63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},
{99,20,8,20,99},{7,8,112,8,7},{97,81,73,69,67},{0,127,65,65,0},
{2,4,8,16,32},{0,65,65,127,0},{4,2,1,2,4},{64,64,64,64,64},
{0,1,2,4,0},{32,84,84,84,120},{127,72,68,68,56},{56,68,68,68,32},
{56,68,68,72,127},{56,84,84,84,24},{8,126,9,1,2},{12,82,82,82,62},
{127,8,4,4,120},{0,68,125,64,0},{32,64,68,61,0},{127,16,40,68,0},
{0,65,127,64,0},{124,4,24,4,120},{124,8,4,4,120},{56,68,68,68,56},
{124,20,20,20,8},{8,20,20,24,124},{124,8,4,4,8},{72,84,84,84,32},
{4,63,68,64,32},{60,64,64,32,124},{28,32,64,32,28},{60,64,48,64,60},
{68,40,16,40,68},{12,80,80,80,60},{68,100,84,76,68}
};

static void set_col(SDL_Renderer*r,Uint32 rgba){
    SDL_SetRenderDrawColor(r,(rgba>>24)&0xff,(rgba>>16)&0xff,(rgba>>8)&0xff,rgba&0xff);
}
static void fill_rect(SDL_Renderer*r,int x,int y,int w,int h,Uint32 c){
    set_col(r,c);SDL_Rect rc={x,y,w,h};SDL_RenderFillRect(r,&rc);
}
static void draw_border(SDL_Renderer*r,int x,int y,int w,int h,Uint32 c){
    set_col(r,c);SDL_Rect rc={x,y,w,h};SDL_RenderDrawRect(r,&rc);
}
static void draw_char(SDL_Renderer*r,int x,int y,char c,int sc,Uint32 col){
    int i=(unsigned char)c-32;
    if(i<0||i>=(int)(sizeof(F57)/5))return;
    set_col(r,col);
    for(int cx=0;cx<5;cx++){
        Uint8 b=F57[i][cx];
        for(int ry=0;ry<7;ry++) {
            if(b&(1<<ry)){SDL_Rect rc={x+cx*sc,y+ry*sc,sc,sc};SDL_RenderFillRect(r,&rc);}
        }
    }
}
static void draw_str(SDL_Renderer*r,int x,int y,const char*s,int sc,Uint32 col){
    while(*s){draw_char(r,x,y,*s,sc,col);x+=6*sc;s++;}
}
static int str_w(const char*s,int sc){return(int)strlen(s)*6*sc;}
static bool in_rect(int mx,int my,SDL_Rect b){return mx>=b.x&&mx<b.x+b.w&&my>=b.y&&my<b.y+b.h;}

#define BID_8G    0
#define BID_RGB   1
#define BID_RGBA  2
#define BID_16G   3
#define BID_RW    4
#define BID_PLAY  5
#define BID_FF    6
#define BID_MUTE  7
#define BID_HELP  8
#define BID_COUNT 9

typedef struct { SDL_Rect r; const char*lbl; int id; } Btn;

typedef struct {
    Uint8 *data;
    size_t size;
    char name[256];
    SDL_atomic_t playing;
    double pos;
    double speed;
    SDL_AudioDeviceID adev;
    volatile size_t audio_pos;
    double audio_accum;
} PlaybackState;

typedef struct {
    int show_help;
    char flash[160];
    Uint32 flash_until;
    int flash_ok;
    SDL_Rect vol_slider;
    int vol_drag;
    float vol_norm;
    int muted;
    char tooltip[128];
    int tooltip_x, tooltip_y;
    int scr_drag, scr_drag_y;
    double scr_drag_pos;
    Btn btns[BID_COUNT];
    int hov[BID_COUNT];
    SDL_Rect speed_pos;
} UIState;

typedef struct {
    PixelMode mode;
    int view_w_px;
    SDL_Texture *wf_tex;
    int wf_tex_w, wf_tex_h;
    int scroll_y;
} RenderState;

typedef struct {
    PlaybackState pb;
    UIState ui;
    RenderState rnd;
} App;

typedef struct {
    int x, y, w, h;
    size_t base, stride;
    int rows;
} Viewport;

static void do_flash(App *app, const char*m, bool ok, int ms){
    snprintf(app->ui.flash,sizeof(app->ui.flash),"%s",m);
    app->ui.flash_ok=ok;app->ui.flash_until=SDL_GetTicks()+ms;
}

static void audio_cb(void*ud, Uint8*stream, int len){
    App *app = (App*)ud;
    Sint16* out = (Sint16*)stream;
    int n = len/2;

    if(!app->pb.data || app->pb.size==0 || !SDL_AtomicGet(&app->pb.playing)){
        SDL_memset(stream, 0, len);
        return;
    }

    float gain = app->ui.muted ? 0.0f : (app->ui.vol_norm * MAX_GAIN);
    double speed = app->pb.speed;
    if(speed < 0.001) speed = 0.001;

    for(int i=0; i<n; i++){
        double exact_pos = (double)app->pb.audio_pos + app->pb.audio_accum;
        size_t p1 = (size_t)exact_pos;
        size_t p2 = p1 + 1;
        float frac = exact_pos - (double)p1;

        Sint16 s1 = (p1 < app->pb.size) ? (Sint16)(((int)app->pb.data[p1] - 128) << 8) : 0;
        Sint16 s2 = (p2 < app->pb.size) ? (Sint16)(((int)app->pb.data[p2] - 128) << 8) : 0;
        Sint16 s = (Sint16)(s1 + (s2 - s1) * frac);
        out[i] = (Sint16)(s * gain);

        app->pb.audio_accum += speed;
        int steps = (int)app->pb.audio_accum;
        if(steps > 0) {
            app->pb.audio_pos += steps;
            app->pb.audio_accum -= steps;
        }
    }
}

static void audio_init(App *app){
    SDL_AudioSpec want={0},got;
    want.freq=AUDIO_RATE;want.format=AUDIO_S16SYS;
    want.channels=1;want.samples=AUDIO_BUF;want.callback=audio_cb;
    want.userdata = app;
    app->pb.adev=SDL_OpenAudioDevice(NULL,0,&want,&got,0);
    if(app->pb.adev)SDL_PauseAudioDevice(app->pb.adev,0);
}

static void load_file(App *app, const char*path){
    FILE*fp=fopen(path,"rb");
    if(!fp){do_flash(app,"Cannot open file",false,3000);return;}
    fseek(fp,0,SEEK_END);long sz=ftell(fp);fseek(fp,0,SEEK_SET);
    if(sz<0){fclose(fp);do_flash(app,"Cannot read file",false,3000);return;}
    Uint8*nd=(Uint8*)malloc(sz?(size_t)sz:1);
    if(!nd){fclose(fp);do_flash(app,"Out of memory",false,3000);return;}
    if(sz>0 && fread(nd,1,(size_t)sz,fp)!=(size_t)sz){
        free(nd);fclose(fp);do_flash(app,"Read error",false,3000);return;
    }
    fclose(fp);
    if(app->pb.data)free(app->pb.data);
    app->pb.data=nd; app->pb.size=(size_t)sz;
    
    const char*b=path;
    const char*sl=strrchr(path,'/'),*bs=strrchr(path,'\\');
    if(sl&&sl>=b)b=sl+1;if(bs&&bs>=b)b=bs+1;
    snprintf(app->pb.name,sizeof(app->pb.name),"%s",b);
    
    app->pb.pos=0;
    app->rnd.scroll_y=0;
    SDL_AtomicSet(&app->pb.playing, 1);
    app->pb.audio_pos = 0;
    app->pb.audio_accum=0.0;
    
    char buf[64];
    if(app->pb.size>=(size_t)(1<<20)) snprintf(buf,sizeof(buf),"Loaded %.1f MB",(double)app->pb.size/(1<<20));
    else if(app->pb.size>=1024)       snprintf(buf,sizeof(buf),"Loaded %.1f KB",(double)app->pb.size/1024);
    else                              snprintf(buf,sizeof(buf),"Loaded %zu B",app->pb.size);
    do_flash(app,buf,true,2500);
}

static size_t row_stride(App *app){return (size_t)app->rnd.view_w_px * MODE_BPP[app->rnd.mode];}

static void decode_pixel(App *app, size_t byte_idx, Uint8 *r, Uint8 *g, Uint8 *b){
    Uint8 rv=0,gv=0,bv=0;
    switch(app->rnd.mode){
        case MODE_8G: if(byte_idx<app->pb.size)rv=gv=bv=app->pb.data[byte_idx]; break;
        case MODE_RGB: case MODE_RGBA:
            if(byte_idx+0<app->pb.size)rv=app->pb.data[byte_idx+0];
            if(byte_idx+1<app->pb.size)gv=app->pb.data[byte_idx+1];
            if(byte_idx+2<app->pb.size)bv=app->pb.data[byte_idx+2]; break;
        case MODE_16G:{
            Uint16 v=0;
            if(byte_idx+0<app->pb.size)v =(Uint16)(app->pb.data[byte_idx+0]<<8);
            if(byte_idx+1<app->pb.size)v|=(Uint16)(app->pb.data[byte_idx+1]);
            rv=gv=bv=(Uint8)(v>>8); break;
        }
        default: break;
    }
    *r=rv; *g=gv; *b=bv;
}

static void draw_btn(SDL_Renderer*r, SDL_Rect b, const char*lbl, bool hov, bool active){
    Uint32 bg=active?C_ACT_BG:(hov?C_BTN_HOV:C_BTN);
    Uint32 brd=active?C_ACT:(hov?0x8888AAFF:C_BTN_BRD);
    Uint32 fg=active?C_ACT:C_TEXT;
    fill_rect(r,b.x,b.y,b.w,b.h,bg);
    draw_border(r,b.x,b.y,b.w,b.h,brd);
    int tw=str_w(lbl,2);
    draw_str(r,b.x+(b.w-tw)/2,b.y+(b.h-14)/2,lbl,2,fg);
}

static void layout_all(App *app, int ww, int wh){
    UIState *ui = &app->ui;
    int rowA_y = wh-BOT_H+8;
    int rowB_y = rowA_y+SB_H+8;

    int x=10;
    for(int m=0;m<4;m++){
        int w=str_w(MODE_NAMES[m],2)+18;
        ui->btns[BID_8G+m]=(Btn){{x,rowA_y,w,SB_H},MODE_NAMES[m],BID_8G+m};
        x+=w+6;
    }

    int vol_label_w = str_w("VOL",2);
    int vol_label_x = x+14;
    int slider_w = 90, slider_h = 10;
    ui->vol_slider.x = vol_label_x+vol_label_w+8;
    ui->vol_slider.y = rowA_y+(SB_H-slider_h)/2;
    ui->vol_slider.w = slider_w;
    ui->vol_slider.h = slider_h;

    int help_w = str_w("?",2)+18;
    int mute_w = str_w("MUTE",2)+18;
    int rx = ww-10;
    rx -= help_w; ui->btns[BID_HELP]=(Btn){{rx,rowA_y,help_w,SB_H},"?",BID_HELP};
    rx -= 8;
    rx -= mute_w; ui->btns[BID_MUTE]=(Btn){{rx,rowA_y,mute_w,SB_H},"MUTE",BID_MUTE};

    int rw_w=str_w("<<",2)+18;
    int ff_w=str_w(">>",2)+18;
    int play_w=str_w("PAUSE",2)+22;
    int total=rw_w+8+play_w+8+ff_w;
    int cx=(ww-total)/2; if(cx<10)cx=10;
    ui->btns[BID_RW]  =(Btn){{cx,rowB_y,rw_w,  SB_H},"<<",   BID_RW};   cx+=rw_w+8;
    ui->btns[BID_PLAY]=(Btn){{cx,rowB_y,play_w,SB_H},"PLAY",BID_PLAY}; cx+=play_w+8;
    ui->btns[BID_FF]  =(Btn){{cx,rowB_y,ff_w,  SB_H},">>",  BID_FF};

    ui->speed_pos.x = ui->btns[BID_FF].r.x+ui->btns[BID_FF].r.w+14;
    ui->speed_pos.y = rowB_y;
}

static void render_waterfall(SDL_Renderer*ren, App *app, Viewport *vp){
    RenderState *rnd = &app->rnd;
    PlaybackState *pb = &app->pb;

    if(!rnd->wf_tex || rnd->wf_tex_w != vp->w || rnd->wf_tex_h != vp->h){
        if(rnd->wf_tex) SDL_DestroyTexture(rnd->wf_tex);
        rnd->wf_tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, vp->w, vp->h);
        rnd->wf_tex_w = vp->w; rnd->wf_tex_h = vp->h;
    }

    void *pixels;
    int pitch;
    SDL_LockTexture(rnd->wf_tex, NULL, &pixels, &pitch);
    Uint32 *px = (Uint32*)pixels;

    for(int row=0; row<vp->rows; row++){
        int rfb=vp->rows-1-row;
        int yp=row*PIXEL_H-rnd->scroll_y;
        if(yp+PIXEL_H<=0||yp>=vp->h)continue;
        
        float age=1.0f-(float)rfb/(float)(vp->rows>1?vp->rows-1:1);
        long long bs=(long long)vp->base-(long long)rfb*(long long)vp->stride;
        
        if(bs<0){
            for(int py=yp; py<yp+PIXEL_H && py<vp->h; py++){
                if(py<0) continue;
                for(int x=0; x<vp->w; x++) px[py * (pitch/4) + x] = 0xFF161414;
            }
            continue;
        }

        for(int x=0; x<vp->w; x++){
            int src_col = (int)((double)x * rnd->view_w_px / (double)vp->w);
            if(src_col >= rnd->view_w_px) src_col = rnd->view_w_px - 1;
            
            size_t bi = (size_t)bs + (size_t)src_col * MODE_BPP[rnd->mode];
            Uint8 r, g, b;
            decode_pixel(app, bi, &r, &g, &b);
            
            r = (Uint8)(r * age);
            g = (Uint8)(g * age);
            b = (Uint8)(b * age);

            Uint32 color = r | (g << 8) | (b << 16) | (0xFF << 24);
            for(int py=yp; py<yp+PIXEL_H && py<vp->h; py++){
                if(py<0) continue;
                px[py * (pitch/4) + x] = color;
            }
        }
    }
    SDL_UnlockTexture(rnd->wf_tex);
    
    SDL_Rect dst = {vp->x, vp->y, vp->w, vp->h};
    SDL_RenderCopy(ren, rnd->wf_tex, NULL, &dst);

    set_col(ren, 0x28FFFFFF);
    SDL_RenderDrawLine(ren, vp->x, vp->y+vp->h-PIXEL_H, vp->x+vp->w, vp->y+vp->h-PIXEL_H);
}

static void render_progress(SDL_Renderer*ren, App *app, Viewport *vp){
    if(app->pb.size>0){
        int bx=vp->x, by=vp->y+vp->h, bw=vp->w, bh=2;
        fill_rect(ren,bx,by,bw,bh,C_BTN);
        int filled=(int)((double)bw*(app->pb.pos/(double)app->pb.size));
        if(filled>bw)filled=bw;
        fill_rect(ren,bx,by,filled,bh,C_ACT);
    }
}

static void render_minimap(SDL_Renderer*ren, App *app, Viewport *vp, int ww){
    int sx=ww-SCR_W, sy=vp->y, sh=vp->h;
    fill_rect(ren,sx,sy,SCR_W,sh,0x1E1E20FF);
    
    if(app->pb.size>0 && vp->stride>0){
        for(int y=0; y<sh; y++){
            double frac = (double)y / (double)sh;
            size_t offset = (size_t)(frac * (double)app->pb.size);
            offset = (offset / vp->stride) * vp->stride;
            Uint8 r,g,b;
            decode_pixel(app, offset, &r, &g, &b);
            SDL_SetRenderDrawColor(ren, r, g, b, 255);
            SDL_RenderDrawLine(ren, sx+2, sy+y, sx+SCR_W-3, sy+y);
        }
    }
}

static void render_scrollbar(SDL_Renderer*ren, App *app, Viewport *vp, int ww){
    if(app->pb.size==0) return;
    int sx=ww-SCR_W, sy=vp->y, sh=vp->h;
    int thumb_h=sh/8; if(thumb_h<24)thumb_h=24; if(thumb_h>sh)thumb_h=sh;
    double frac=app->pb.pos/(double)app->pb.size;
    int thumb_y=sy+(int)(frac*(sh-thumb_h));
    if(thumb_y<sy)thumb_y=sy;
    if(thumb_y+thumb_h>sy+sh)thumb_y=sy+sh-thumb_h;
    
    int mx,my; SDL_GetMouseState(&mx,&my);
    bool on_thumb=(mx>=sx&&mx<sx+SCR_W&&my>=thumb_y&&my<thumb_y+thumb_h);
    fill_rect(ren,sx+1,thumb_y,SCR_W-2,thumb_h,(app->ui.scr_drag||on_thumb)?0x6060AAFF:0x44444AFF);
    draw_border(ren,sx+1,thumb_y,SCR_W-2,thumb_h,0xAAAAAAFF);
}

static void update_tooltip(App *app, Viewport *vp){
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    if(mx < vp->w && my >= vp->y && my < vp->y+vp->h && app->pb.data) {
        int src_col = (int)((double)mx * app->rnd.view_w_px / (double)vp->w);
        if(src_col >= app->rnd.view_w_px) src_col = app->rnd.view_w_px - 1;
        int rfb = (vp->h - 1 - (my - vp->y) + app->rnd.scroll_y) / PIXEL_H;
        size_t byte_off = vp->base - (size_t)rfb * vp->stride + (size_t)src_col * MODE_BPP[app->rnd.mode];
        
        if(byte_off < app->pb.size) {
            int bpp = MODE_BPP[app->rnd.mode];
            char hex[32] = {0};
            for(int i=0; i<bpp && byte_off+i < app->pb.size; i++) {
                sprintf(hex + strlen(hex), "%02X ", app->pb.data[byte_off + i]);
            }
            snprintf(app->ui.tooltip, sizeof(app->ui.tooltip), "Off: 0x%08zX | %s", byte_off, hex);
            app->ui.tooltip_x = mx + 12;
            app->ui.tooltip_y = my - 8;
        } else {
            app->ui.tooltip[0] = 0;
        }
    } else {
        app->ui.tooltip[0] = 0;
    }
}

static void draw_waterfall_viewport(SDL_Renderer*ren, App *app, int ww, int wh){
    int vx=0, vy=BAR_H, vw=ww-SCR_W, vh=wh-BAR_H-BOT_H;
    if(vh<=0||vw<=0) return;

    size_t stride=row_stride(app);
    if(stride>0){
        double rowf = app->pb.pos/(double)stride;
        double frac = rowf - floor(rowf);
        app->rnd.scroll_y = (int)(frac*(double)PIXEL_H);
    } else {
        app->rnd.scroll_y = 0;
    }

    int rows=vh/PIXEL_H+2;
    size_t base=(size_t)app->pb.pos;
    base=(stride>0)?(base/stride)*stride:0;

    Viewport vp = {vx, vy, vw, vh, base, stride, rows};

    render_waterfall(ren, app, &vp);
    render_progress(ren, app, &vp);
    render_minimap(ren, app, &vp, ww);
    render_scrollbar(ren, app, &vp, ww);
    update_tooltip(app, &vp);
}

static void draw_volume(SDL_Renderer*ren, App *app, int rowA_y){
    UIState *ui = &app->ui;
    int label_x = ui->btns[BID_16G].r.x+ui->btns[BID_16G].r.w+14;
    Uint32 lc = ui->muted? C_MUTED : C_TEXT;
    draw_str(ren,label_x,rowA_y+(SB_H-14)/2,"VOL",2,lc);

    fill_rect(ren,ui->vol_slider.x,ui->vol_slider.y,ui->vol_slider.w,ui->vol_slider.h,C_BTN);
    draw_border(ren,ui->vol_slider.x-1,ui->vol_slider.y-1,ui->vol_slider.w+2,ui->vol_slider.h+2,C_BTN_BRD);
    int filled=(int)(ui->vol_norm*ui->vol_slider.w);
    fill_rect(ren,ui->vol_slider.x,ui->vol_slider.y,filled,ui->vol_slider.h,ui->muted?C_MUTED:C_ACT);
    int knob_x = ui->vol_slider.x+filled-3;
    fill_rect(ren,knob_x,ui->vol_slider.y-3,6,ui->vol_slider.h+6,ui->muted?C_MUTED:C_TEXT);
}

static void draw_help(SDL_Renderer*r,int ww,int wh){
    SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r,0x14,0x14,0x16,0xD0);
    SDL_Rect full={0,0,ww,wh}; SDL_RenderFillRect(r,&full);
    SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);

    int pw=380,ph=300,px=(ww-pw)/2,py=(wh-ph)/2;
    fill_rect(r,px,py,pw,ph,C_BAR);
    draw_border(r,px,py,pw,ph,C_DIV);
    draw_border(r,px+1,py+1,pw-2,ph-2,0x444448FF);

    const char*title="Controls & Shortcuts";
    draw_str(r,px+(pw-str_w(title,2))/2,py+14,title,2,C_TEXT);
    fill_rect(r,px+12,py+34,pw-24,1,C_DIV);

    static const char*keys[][2]={
        {"Space",       "Play / Pause"},
        {"R",           "Restart"},
        {"M",           "Mute / Unmute"},
        {"Up / Down",   "Speed x2 / x0.5"},
        {"Wheel",       "Scroll / Seek"},
        {"Ctrl+Wheel",  "Zoom In / Out"},
        {"Scroll Drag", "Click track to jump"},
        {"Drag & Drop", "Load any file"},
    };
    int ny=py+46;
    for(int i=0;i<8;i++){
        draw_str(r,px+18,ny,keys[i][0],1,C_ACT);
        draw_str(r,px+160,ny,keys[i][1],1,C_MUTED);
        ny+=22;
    }
    const char*cl="[?] or [Esc] to close";
    draw_str(r,px+(pw-str_w(cl,1))/2,py+ph-18,cl,1,C_MUTED);
}

static void handle_keyboard(App *app, SDL_Event *ev){
    if(app->ui.show_help){
        if(ev->key.keysym.sym==SDLK_ESCAPE||ev->key.keysym.sym==SDLK_SLASH
           ||ev->key.keysym.sym==SDLK_QUESTION||ev->key.keysym.sym==SDLK_h)
            app->ui.show_help=0;
        return;
    }
    switch(ev->key.keysym.sym){
        case SDLK_ESCAPE: case SDLK_q: 
            ev->type = SDL_QUIT; // Hacky but clean for main loop exit
            break;
        case SDLK_SPACE:  SDL_AtomicSet(&app->pb.playing, !SDL_AtomicGet(&app->pb.playing)); break;
        case SDLK_r:      
            app->pb.pos=0; app->pb.audio_pos=0; app->pb.audio_accum=0.0; 
            SDL_AtomicSet(&app->pb.playing, 1); 
            break;
        case SDLK_m:      app->ui.muted=!app->ui.muted; break;
        case SDLK_h: case SDLK_SLASH: app->ui.show_help=!app->ui.show_help; break;
        case SDLK_UP:   app->pb.speed*=2.0;if(app->pb.speed>32)app->pb.speed=32; break;
        case SDLK_DOWN: app->pb.speed*=0.5;if(app->pb.speed<0.125)app->pb.speed=0.125; break;
    }
}

static void handle_mouse(App *app, SDL_Event *ev, int ww, int wh){
    if(ev->type==SDL_MOUSEBUTTONDOWN && ev->button.button==SDL_BUTTON_LEFT){
        int bx=ev->button.x, by=ev->button.y;
        if(app->ui.show_help){ app->ui.show_help=0; return; }
        
        if(in_rect(bx, by, app->ui.vol_slider)){
            app->ui.vol_drag=1;
            float v=(float)(bx-app->ui.vol_slider.x)/(float)app->ui.vol_slider.w;
            if(v<0)v=0; if(v>1)v=1;
            app->ui.vol_norm=v;
            return;
        }

        int vy=BAR_H, vh=wh-BAR_H-BOT_H, sx=ww-SCR_W;
        if(bx>=sx && app->pb.size>0){
            int sh=vh;
            int thumb_h=sh/8; if(thumb_h<24)thumb_h=24;
            int thumb_y=vy+(int)((app->pb.pos/(double)app->pb.size)*(sh-thumb_h));
            
            if(by>=thumb_y && by<thumb_y+thumb_h){
                app->ui.scr_drag=1; app->ui.scr_drag_y=by; app->ui.scr_drag_pos=app->pb.pos;
            } else {
                double nf=(double)(by-vy)/(double)sh;
                if(nf<0)nf=0;if(nf>1)nf=1;
                app->pb.pos=nf*(double)app->pb.size;
                app->pb.audio_pos=(size_t)app->pb.pos;
                app->pb.audio_accum=0.0;
                app->ui.scr_drag=1; app->ui.scr_drag_y=by; app->ui.scr_drag_pos=app->pb.pos;
            }
            return;
        }

        if(app->ui.hov[BID_PLAY]) SDL_AtomicSet(&app->pb.playing, !SDL_AtomicGet(&app->pb.playing));
        if(app->ui.hov[BID_RW]){
            size_t s=row_stride(app); app->pb.pos-=(double)s*30;
            if(app->pb.pos<0)app->pb.pos=0;
            app->pb.audio_pos=(size_t)app->pb.pos; app->pb.audio_accum=0.0;
        }
        if(app->ui.hov[BID_FF] && app->pb.size){
            size_t s=row_stride(app); app->pb.pos+=(double)s*30;
            if(app->pb.pos>(double)app->pb.size)app->pb.pos=(double)app->pb.size;
            app->pb.audio_pos=(size_t)app->pb.pos; app->pb.audio_accum=0.0;
        }
        if(app->ui.hov[BID_MUTE]) app->ui.muted=!app->ui.muted;
        if(app->ui.hov[BID_HELP]) app->ui.show_help=!app->ui.show_help;
        for(int m=0;m<4;m++) if(app->ui.hov[BID_8G+m]) app->rnd.mode=(PixelMode)m;
    }

    if(ev->type==SDL_MOUSEBUTTONUP && ev->button.button==SDL_BUTTON_LEFT){
        app->ui.scr_drag=0;
        app->ui.vol_drag=0;
    }

    if(ev->type==SDL_MOUSEMOTION){
        if(app->ui.vol_drag){
            float v=(float)(ev->motion.x-app->ui.vol_slider.x)/(float)app->ui.vol_slider.w;
            if(v<0)v=0; if(v>1)v=1;
            app->ui.vol_norm=v;
        }
        if(app->ui.scr_drag && app->pb.size>0){
            int sh=wh-BAR_H-BOT_H;
            int thumb_h=sh/8; if(thumb_h<24)thumb_h=24;
            if(sh-thumb_h>0){
                double delta=(double)(ev->motion.y-app->ui.scr_drag_y)/(double)(sh-thumb_h);
                app->pb.pos=app->ui.scr_drag_pos+delta*(double)app->pb.size;
                if(app->pb.pos<0)app->pb.pos=0;
                if(app->pb.pos>(double)app->pb.size)app->pb.pos=(double)app->pb.size;
                app->pb.audio_pos=(size_t)app->pb.pos;
                app->pb.audio_accum=0.0;
            }
        }
    }

    if(ev->type==SDL_MOUSEWHEEL && !app->ui.show_help){
        if(SDL_GetModState() & KMOD_CTRL) {
            app->rnd.view_w_px -= ev->wheel.y * 16;
            if(app->rnd.view_w_px < 32) app->rnd.view_w_px = 32;
            if(app->rnd.view_w_px > 2048) app->rnd.view_w_px = 2048;
        } else {
            size_t s=row_stride(app);
            app->pb.pos-=(double)ev->wheel.y*s*5;
            if(app->pb.pos<0)app->pb.pos=0;
            if(app->pb.pos>(double)app->pb.size)app->pb.pos=(double)app->pb.size;
            app->pb.audio_pos=(size_t)app->pb.pos;
            app->pb.audio_accum=0.0;
        }
    }
}

static void update(App *app){
    if(SDL_AtomicGet(&app->pb.playing) && app->pb.data && app->pb.size>0 && !app->ui.show_help){
        app->pb.pos=(double)app->pb.audio_pos;
        if(app->pb.audio_pos>=app->pb.size){
            app->pb.pos=(double)app->pb.size;
            SDL_AtomicSet(&app->pb.playing, 0);
            do_flash(app,"End of file",true,2000);
        }
    }
}

static void render(SDL_Renderer*ren, App *app, int ww, int wh){
    set_col(ren,C_BG); SDL_RenderClear(ren);

    fill_rect(ren,0,0,ww,BAR_H,C_BAR);
    fill_rect(ren,0,BAR_H-1,ww,1,C_DIV);
    
    if(app->pb.name[0]){
        int tw=str_w(app->pb.name,2),tx=(ww-tw)/2; if(tx<8)tx=8;
        draw_str(ren,tx,BAR_H/2-7,app->pb.name,2,C_TEXT);
        
        char sz[32];
        if(app->pb.size>=(size_t)(1<<20)) snprintf(sz,sizeof(sz),"%.1f MB",(double)app->pb.size/(1<<20));
        else if(app->pb.size>=1024)       snprintf(sz,sizeof(sz),"%.1f KB",(double)app->pb.size/1024);
        else                              snprintf(sz,sizeof(sz),"%zu B",app->pb.size);
        draw_str(ren,ww-str_w(sz,1)-10,BAR_H/2-3,sz,1,C_MUTED);
        
        char zbuf[32];
        snprintf(zbuf, sizeof(zbuf), "W:%d", app->rnd.view_w_px);
        draw_str(ren, 8, BAR_H/2-3, zbuf, 1, C_MUTED);
    }

    draw_waterfall_viewport(ren, app, ww, wh);
    
    if(!app->pb.data){
        const char*m="Drop any file onto the window";
        draw_str(ren,(ww-str_w(m,1))/2,wh/2-3,m,1,C_MUTED);
    }

    int rowA_y = wh-BOT_H+8;
    fill_rect(ren,0,wh-BOT_H,ww,BOT_H,C_BAR);
    fill_rect(ren,0,wh-BOT_H,ww,1,C_DIV);

    for(int m=0;m<4;m++)
        draw_btn(ren, app->ui.btns[BID_8G+m].r, MODE_NAMES[m], app->ui.hov[BID_8G+m], (app->rnd.mode==m));
        
    draw_volume(ren, app, rowA_y);
    draw_btn(ren, app->ui.btns[BID_MUTE].r, "MUTE", app->ui.hov[BID_MUTE], app->ui.muted);
    draw_btn(ren, app->ui.btns[BID_HELP].r, "?",    app->ui.hov[BID_HELP], app->ui.show_help);

    draw_btn(ren, app->ui.btns[BID_RW].r,  "<<", app->ui.hov[BID_RW],  false);
    draw_btn(ren, app->ui.btns[BID_PLAY].r, SDL_AtomicGet(&app->pb.playing)?"PAUSE":"PLAY", app->ui.hov[BID_PLAY], SDL_AtomicGet(&app->pb.playing));
    draw_btn(ren, app->ui.btns[BID_FF].r,  ">>", app->ui.hov[BID_FF],  false);

    char spbuf[16]; 
    snprintf(spbuf,sizeof(spbuf),"x%.3g",app->pb.speed);
    draw_str(ren, app->ui.speed_pos.x, app->ui.speed_pos.y+(SB_H-7)/2, spbuf, 1, C_MUTED);

    if(SDL_GetTicks() < app->ui.flash_until){
        Uint32 bc=app->ui.flash_ok?C_GREEN_BG:C_RED_BG, tc=app->ui.flash_ok?C_GREEN:C_RED;
        int fw=str_w(app->ui.flash,1)+24, fh=22;
        int fx=ww-fw-10, fy=wh-BOT_H-fh-8;
        fill_rect(ren,fx,fy,fw,fh,bc);
        draw_border(ren,fx,fy,fw,fh,tc);
        draw_str(ren,fx+12,fy+(fh-7)/2,app->ui.flash,1,tc);
    }

    if(app->ui.tooltip[0]) {
        int tw = str_w(app->ui.tooltip, 1) + 12;
        int th = 16;
        int tx = app->ui.tooltip_x;
        int ty = app->ui.tooltip_y - th;
        
        if(tx + tw > ww - 5) tx = ww - tw - 5;
        if(ty < BAR_H + 2) ty = app->ui.tooltip_y + 12;
        
        fill_rect(ren, tx, ty, tw, th, 0x18181AFF);
        draw_border(ren, tx, ty, tw, th, 0x44444AFF);
        draw_str(ren, tx+6, ty+4, app->ui.tooltip, 1, C_ACT);
    }

    if(app->ui.show_help) draw_help(ren, ww, wh);

    SDL_RenderPresent(ren);
}

int main(int argc, char**argv){
    App app = {0};
    app.rnd.view_w_px = 128;
    app.ui.vol_norm = 0.4f;
    app.pb.speed = 1.0;
    SDL_AtomicSet(&app.pb.playing, 0);

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    SDL_Window*win=SDL_CreateWindow("Bytefall",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIN_W,WIN_H,
        SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_SetWindowMinimumSize(win,MIN_W,MIN_H);
    SDL_Renderer*ren=SDL_CreateRenderer(win,-1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);

    audio_init(&app);
    if(argc>=2) load_file(&app, argv[1]);

    int running=1; 
    SDL_Event ev;
    
    while(running){
        int mx, my; 
        SDL_GetMouseState(&mx, &my);
        int ww, wh; 
        SDL_GetWindowSize(win, &ww, &wh);

        layout_all(&app, ww, wh);
        for(int i=0; i<BID_COUNT; i++)
            app.ui.hov[i] = in_rect(mx, my, app.ui.btns[i].r);

        while(SDL_PollEvent(&ev)){
            if(ev.type == SDL_QUIT) running = 0;
            else if(ev.type == SDL_DROPFILE){ load_file(&app, ev.drop.file); SDL_free(ev.drop.file); }
            else if(ev.type == SDL_KEYDOWN) handle_keyboard(&app, &ev);
            else handle_mouse(&app, &ev, ww, wh);
        }

        update(&app);
        render(ren, &app, ww, wh);
        SDL_Delay(8);
    }

    if(app.pb.adev) SDL_CloseAudioDevice(app.pb.adev);
    if(app.pb.data) free(app.pb.data);
    if(app.rnd.wf_tex) SDL_DestroyTexture(app.rnd.wf_tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
