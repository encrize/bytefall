# BYTEFALL
![Bytefall preview](preview.jpg)
Bytefall is a minimal, highly optimized binary file visualizer and raw audio player. It interprets any file as structured pixel data, rendering it as a scrolling waterfall, and simultaneously plays the raw byte values back as an audio signal.
## Features
*    High-Performance Rendering: Direct texture memory manipulation for smooth 60+ FPS visualization.
*    Pixel Modes: 8-bit Grayscale, 16-bit Grayscale, RGB, and RGBA.
*    Dynamic Zoom: Inspect file structures closely or zoom out for an overview.
*    Raw Audio Engine: Plays byte data as audio with linear interpolation for clean sound at any speed.
*    Analysis Tools: Real-time hex tooltips, offset tracking, and a colorized minimap scrollbar.
*    Zero Dependencies: Compiles to a single, static executable.
## Build
### Windows
> Requires MSYS2 mingw-w64.
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2
```
```bash
 gcc -O3 -o bytefall.exe bytefall.c -static -lmingw32 -lSDL2main -lSDL2 -mwindows -lwinmm -limm32 -lole32 -loleaut32 -lversion -lpropsys -lsetupapi -lcfgmgr32
```
### Linux
> Requires SDL2
```bash
sudo apt install libsdl2-dev
```
```bash
gcc bytefall.c -o bytefall $(sdl2-config --cflags --libs) -lm
```
### macOS
> Requires SDL2 via Homebrew
```bash
brew install sdl2
```
```bash
gcc bytefall.c -o bytefall $(sdl2-config --cflags --libs) -lm
```

## Usage
Drag and drop any file onto the exe/window or pass it as an argument:
    ./bytefall myfile.bin

## Pixel modes
| Mode | Bytes per pixel |
| :---:| :---: |
| 8G | 1 - grayscale | 
| RGB | 3 - color (default) | 
| RGBA | 4 - color with alpha | 
| 16G | 2 - 16-bit grayscale | 

## Controls
| Key | Action |
| :---: | :---: |
| `Space` | Play / Pause |
| `R` | Restart |
| `M` | Mute |
| `↑` / `↓` | Speed |
| `?` | Help |
| `esc` | Exit |
| `Mouse wheel` | Scroll |
| `Ctrl + Mouse Wheel` | Zoom |

## IMG2BIN
You can create images like that using [img2bin](https://github.com/encrize/image-to-bin)
![IMG2BIN](previewBIN.png)

made by encrize , my website - https://encrize.vip
