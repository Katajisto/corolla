#include "microui.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "atlas.inl"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_FPoint points[500];

int text_width(mu_Font font, const char *text, int len) {
    int res = 0;
    for (const char *p = text; *p && len--; p++) {
        if ((*p & 0xc0) == 0x80) { continue; }
            int chr = mu_min((unsigned char) *p, 127);
            res += atlas[ATLAS_FONT + chr].w;
        }
    return res;
}

int text_height(mu_Font font) {
    return 18;
}


int min(int a, int b) {
    return a < b ? a : b;
}

static const char button_map[256] = {
  [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
  [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
  [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
  [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
  [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};


int main(void) {
    mu_Context *ctx = malloc(sizeof(mu_Context));    
    mu_init(ctx);

    SDL_SetAppMetadata("Corolla", "0.0.0", "com.ktjst.corolla");

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Init failed");
        return 1;
    }

    if(!SDL_CreateWindowAndRenderer("Corolla", 1500, 1000, 0, &window, &renderer)) {
        SDL_Log("Window init failed");
        return 1;
    }

    ctx->text_height = text_height;
    ctx->text_width = text_width;

    unsigned char pixels[128*128*4];
    
    int c = 0;
    for(int i = 0; i < 128 * 128; i++) {
        pixels[c] = atlas_texture[i];
        pixels[c+1] = atlas_texture[i];
        pixels[c+2] = atlas_texture[i];
        pixels[c+3] = 255;
        c+=4;
    }
    
    SDL_Surface *sur = SDL_CreateSurfaceFrom(128, 128, SDL_PIXELFORMAT_XRGB8888, pixels, 4 * 128);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, sur);

    bool running = true;
    
    while(running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
          switch (e.type) {
            case SDL_EVENT_QUIT: exit(EXIT_SUCCESS); break;
            case SDL_EVENT_MOUSE_MOTION: mu_input_mousemove(ctx, e.motion.x, e.motion.y); break;
            case SDL_EVENT_MOUSE_WHEEL: mu_input_scroll(ctx, 0, e.wheel.y * -30); break;
            case SDL_EVENT_TEXT_INPUT: mu_input_text(ctx, e.text.text); break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
              int b = button_map[e.button.button & 0xff];
              if (b && e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
              if (b && e.type ==   SDL_EVENT_MOUSE_BUTTON_UP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b);   }
              break;
            }

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
              int c = key_map[e.key.key & 0xff];
              if (c && e.type == SDL_EVENT_KEY_DOWN) { mu_input_keydown(ctx, c); }
              if (c && e.type ==   SDL_EVENT_KEY_UP) { mu_input_keyup(ctx, c);   }
              break;
            }
          }
        }
        

        SDL_FRect rect;

        
        mu_begin(ctx);
        if(mu_begin_window(ctx, "Test", mu_rect(10, 10, 100, 100))) {
            mu_end_window(ctx);
        }
        mu_end(ctx);

        // UI drawing
        mu_Command *cmd = NULL;

        SDL_FRect full;
        full.x = 0;
        full.y = 0;
        full.w = 128;
        full.h = 128;
        
        SDL_FRect full2;
        full2.x = 0;
        full2.y = 0;
        full2.w = 256;
        full2.h = 256;

        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        // SDL_RenderTexture(renderer, tex, &full, &full2);
        
        while(mu_next_command(ctx, &cmd)) {
            if(cmd->type == MU_COMMAND_TEXT) {
                SDL_FRect dest;
                dest.x = cmd->text.pos.x;
                dest.y = cmd->text.pos.y;

                for(const char *p = cmd->text.str; *p; p++) {
                    if((*p & 0xc0) == 0x80) { continue; }
                    int chr = min((unsigned char) *p, 127);
                    mu_Rect src = atlas[ATLAS_FONT + chr];
                    dest.w = src.w;
                    dest.h = src.h;

                    SDL_FRect srcSDL;
                    srcSDL.h = src.h;
                    srcSDL.w = src.w;
                    srcSDL.x = src.x;
                    srcSDL.y = src.y;
                    
                    SDL_RenderTexture(renderer, tex, &srcSDL, &dest);
                    dest.x += dest.w;
                }
            }
            if(cmd->type == MU_COMMAND_RECT) {
                SDL_FRect rect;
                rect.h = cmd->rect.rect.h;
                rect.w = cmd->rect.rect.w;
                rect.x = cmd->rect.rect.x;
                rect.y = cmd->rect.rect.y;

                SDL_SetRenderDrawColor(renderer, cmd->rect.color.r, cmd->rect.color.g, cmd->rect.color.b, cmd->rect.color.a);
                SDL_RenderFillRect(renderer, &rect);
                
            }
            if(cmd->type == MU_COMMAND_ICON) {
                SDL_FRect rect;
                rect.h = cmd->icon.rect.h;
                rect.w = cmd->icon.rect.w;
                rect.x = cmd->icon.rect.x;
                rect.y = cmd->icon.rect.y;

                mu_Rect r = atlas[cmd->icon.id];
                SDL_FRect srect;
                srect.h = r.h;
                srect.w = r.w;
                srect.x = r.x;
                srect.y = r.y;
                
                SDL_RenderTexture(renderer, tex, &srect, &rect);
                
            }
            if(cmd->type == MU_COMMAND_CLIP) {
                
            }
        }
        SDL_RenderPresent(renderer);
        
    }
    
}
