#ifndef SDL3RENDERER
#define SDL3RENDERER
#include "clay.h"
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

void SDL_RenderClayCommands(SDL_Renderer *renderer, Clay_RenderCommandArray *rcommands);

#endif
