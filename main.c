/*
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static TTF_Font *font = NULL;

extern unsigned char tiny_ttf[];
extern unsigned int tiny_ttf_len;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_Color color = { 255, 255, 255, SDL_ALPHA_OPAQUE };
    SDL_Surface *text;
    float font_size = 18.0f; 

    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Hello World", 800, 600, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize SDL_ttf: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Open the font */
    font = TTF_OpenFont("./osifont.ttf", font_size);
    if (!font) {
        SDL_Log("Couldn't open font: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* Create the text */
    text = TTF_RenderText_Blended_Wrapped(font, "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam dui nulla, sodales et molestie ut, posuere nec diam. Nulla neque mauris, eleifend ac tincidunt vel, molestie a nunc. Aliquam euismod diam at sapien rhoncus suscipit. Vivamus est magna, volutpat sed luctus non, molestie interdum nisl. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Praesent quis mi at enim dignissim dignissim id vel sem. In commodo varius convallis. Nam luctus ornare massa, quis bibendum ex congue in. ", 0, color, 800);
    if (text) {
        texture = SDL_CreateTextureFromSurface(renderer, text);
        SDL_DestroySurface(text);
    }
    if (!texture) {
        SDL_Log("Couldn't create text: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_KEY_DOWN ||
        event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    /* Draw the text */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, &(SDL_FRect){300, 0, 800, 100}, &(SDL_FRect){0, 0, 800, 100});
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}