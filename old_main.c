/* clear.c ... */

/*
 * This example code creates an SDL window and renderer, and then clears the
 * window to a different color every frame, so you'll effectively get a window
 * that's smoothly fading between colors.
 *
 * This code is public domain. Feel SDL_free to use it for any purpose!
 */

#include "SDL3/SDL_audio.h"
#include "SDL3/SDL_init.h"
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define NUM_POINTS 1000
#define MIN_PIXELS_PER_SECOND 30  /* move at least this many pixels per second. */
#define MAX_PIXELS_PER_SECOND 60  /* move this many pixels per second at most. */

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static Uint64 last_time = 0;

static SDL_AudioStream *stream = NULL;
static Uint8 *wav_data = NULL;
static Uint32 wav_data_len = 0;

static SDL_AudioStream *stream1 = NULL;
static Uint8 *wav_data1 = NULL;
static Uint32 wav_data_len1 = 0;

static Uint8 *wav_data2 = NULL;
static Uint32 wav_data_len2 = 0;

static Uint8 *wav_data3 = NULL;
static Uint32 wav_data_len3 = 0;

static SDL_FPoint *points_ptr;
static float *point_speeds_ptr;

static SDL_Texture *texture = NULL;
static int texture_width = 0;
static int texture_height = 0;

static SDL_Texture *texture1 = NULL;
static SDL_Texture *texture2 = NULL;

typedef struct {
    int x;
    int y;
    SDL_FRect teto_rect;
    char *dir;
} Person_t;

static Person_t teto;


static const char *hat_state_string(Uint8 state)
{
    switch (state) {
        case SDL_HAT_CENTERED: return "CENTERED";
        case SDL_HAT_UP: return "UP";
        case SDL_HAT_RIGHT: return "RIGHT";
        case SDL_HAT_DOWN: return "DOWN";
        case SDL_HAT_LEFT: return "LEFT";
        case SDL_HAT_RIGHTUP: return "RIGHT+UP";
        case SDL_HAT_RIGHTDOWN: return "RIGHT+DOWN";
        case SDL_HAT_LEFTUP: return "LEFT+UP";
        case SDL_HAT_LEFTDOWN: return "LEFT+DOWN";
        default: break;
    }
    return "UNKNOWN";
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_AudioSpec spec;
    char *wav_path = NULL;
    
    SDL_AudioSpec spec1;
    SDL_AudioSpec spec2;
    SDL_AudioSpec spec3;

    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("some random things", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);


    /* Load the .wav file from wherever the app is being run from. */
    SDL_asprintf(&wav_path, "%sCCCP.wav", SDL_GetBasePath());  /* allocate a string of the full file path */
    if (!SDL_LoadWAV(wav_path, &spec, &wav_data, &wav_data_len)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SDL_free(wav_path);  /* done with this string. */

    /* Load the .wav file from wherever the app is being run from. */
    SDL_asprintf(&wav_path, "%schina.wav", SDL_GetBasePath());  /* allocate a string of the full file path */
    if (!SDL_LoadWAV(wav_path, &spec1, &wav_data1, &wav_data_len1)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SDL_free(wav_path);  /* done with this string. */

        /* Load the .wav file from wherever the app is being run from. */
    SDL_asprintf(&wav_path, "%seff1.wav", SDL_GetBasePath());  /* allocate a string of the full file path */
    if (!SDL_LoadWAV(wav_path, &spec2, &wav_data2, &wav_data_len2)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SDL_free(wav_path);  /* done with this string. */

        /* Load the .wav file from wherever the app is being run from. */
    SDL_asprintf(&wav_path, "%seff2.wav", SDL_GetBasePath());  /* allocate a string of the full file path */
    if (!SDL_LoadWAV(wav_path, &spec3, &wav_data3, &wav_data_len3)) {
        SDL_Log("Couldn't load .wav file: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SDL_free(wav_path);  /* done with this string. */

    /* Create our audio stream in the same format as the .wav file. It'll convert to what the audio hardware wants. */
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!stream) {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    /* Create our audio stream in the same format as the .wav file. It'll convert to what the audio hardware wants. */
    stream1 = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec1, NULL, NULL);
    if (!stream) {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* SDL_OpenAudioDeviceStream starts the device paused. You have to tell it to start! */
    SDL_ResumeAudioStreamDevice(stream);
    SDL_ResumeAudioStreamDevice(stream1);

    points_ptr = SDL_malloc(NUM_POINTS * sizeof(SDL_FPoint));
    point_speeds_ptr = SDL_malloc(NUM_POINTS * sizeof(float));

    if (points_ptr == NULL || point_speeds_ptr == NULL) {
        SDL_Log("Out of memory!");
        
        SDL_free(points_ptr);
        points_ptr = NULL;
        
        SDL_free(point_speeds_ptr);
        point_speeds_ptr = NULL;

        return SDL_APP_FAILURE;
    }

    // for (int i = 0; i < SDL_arraysize(points); i++) {
    //     points[i].x = (SDL_randf() * 640.0f) + 0.0f;
    //     points[i].y = (SDL_randf() * 480.0f) + 0.0f;
    //     // printf("Ok, %d", SDL_randf());
    // }
    teto.x = 20;
    teto.y = 40;
    teto.dir = "none";
    /* set up the data for a bunch of points. */
    for (int i = 0; i < NUM_POINTS; i++) {
        points_ptr[i].x = SDL_randf() * ((float) WINDOW_WIDTH);
        points_ptr[i].y = SDL_randf() * ((float) WINDOW_HEIGHT);
        point_speeds_ptr[i] = MIN_PIXELS_PER_SECOND + (SDL_randf() * (MAX_PIXELS_PER_SECOND - MIN_PIXELS_PER_SECOND));
    }

    last_time = SDL_GetTicks();

    SDL_Surface *surface = NULL;
    char *png_path = NULL;

    /* Textures are pixel data that we upload to the video hardware for fast drawing. Lots of 2D
       engines refer to these as "sprites." We'll do a static texture (upload once, draw many
       times) with data from a png file. */

    /* SDL_Surface is pixel data the CPU can access. SDL_Texture is pixel data the GPU can access.
       Load a .png into a surface, move it to a texture from there. */
    SDL_asprintf(&png_path, "%steto.png", SDL_GetBasePath());  /* allocate a string of the full file path */
    surface = SDL_LoadPNG(png_path);

    if (!surface) {
        SDL_Log("Couldn't load png: %s", SDL_GetError());
        SDL_DestroyTexture(texture);
        texture = NULL;
        return SDL_APP_FAILURE;
    }

    SDL_SDL_free(png_path);  /* done with this, the file is loaded. */

    texture_width = 100;
    texture_height = 100;

    texture1 = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture1) {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_DestroySurface(surface);  /* done with this, the texture has a copy of the pixels now. */

    SDL_asprintf(&png_path, "%steto2.png", SDL_GetBasePath());  /* allocate a string of the full file path */
    surface = SDL_LoadPNG(png_path);

    if (!surface) {
        SDL_Log("Couldn't load png: %s", SDL_GetError());
        SDL_DestroyTexture(texture2);
        texture2 = NULL;
        return SDL_APP_FAILURE;
    }

    SDL_SDL_free(png_path);  /* done with this, the file is loaded. */

    texture2 = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture2) {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_DestroySurface(surface);  /* done with this, the texture has a copy of the pixels now. */


    texture = texture1;

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}


/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    if (event->type == SDL_EVENT_JOYSTICK_ADDED) {
        /* this event is sent for each hotplugged stick, but also each already-connected joystick during SDL_Init(). */
        const SDL_JoystickID which = event->jdevice.which;
        SDL_Joystick *joystick = SDL_OpenJoystick(which);
        if (!joystick) {
            SDL_Log("Joystick #%u add, but not opened: %s", (unsigned int) which, SDL_GetError());
        } else {
            SDL_Log("Joystick #%u ('%s') added", (unsigned int) which, SDL_GetJoystickName(joystick));
        }
    } else if (event->type == SDL_EVENT_JOYSTICK_REMOVED) {
        const SDL_JoystickID which = event->jdevice.which;
        SDL_Joystick *joystick = SDL_GetJoystickFromID(which);
        if (joystick) {
            SDL_CloseJoystick(joystick);  /* the joystick was unplugged. */
        }
        SDL_Log("Joystick #%u removed", (unsigned int) which);

    }else if(event->type == SDL_EVENT_JOYSTICK_AXIS_MOTION){
        if (((event->jaxis.axis == 1 && event->jaxis.value > 0)) && ((teto.y - 100) >=  40)){
            teto.y -= 100;
            teto.dir = "up";
        }
        if (((event->jaxis.axis == 0 && event->jaxis.value > 0)) && ((teto.x + 100) <= WINDOW_WIDTH - 120)){
            teto.x += 100; teto.dir = "right";
        }

    }
    else if (event->type == SDL_EVENT_KEY_DOWN) {
        /* UP arrow increase alpha */
        if ((event->key.key == SDLK_UP) && ((teto.y - 100) >=  40)){
            teto.y -= 100;
            teto.dir = "up";
            texture = texture1;
            SDL_ClearAudioStream(stream);
            SDL_PutAudioStreamData(stream, wav_data, wav_data_len);
        }
        if ((event->key.key == SDLK_DOWN) && ((teto.y + 100) <= WINDOW_HEIGHT - 140 ) )
        {
            texture = texture1;
            teto.y += 100; teto.dir = "down";
            SDL_ClearAudioStream(stream);
            SDL_PutAudioStreamData(stream, wav_data1, wav_data_len1);
        }
        /* DOWN arrow decrease alpha */
        if ((event->key.key == SDLK_RIGHT) && ((teto.x + 100) <= WINDOW_WIDTH - 120)){
            texture = texture2;
            teto.x += 100; teto.dir = "right";
            SDL_ClearAudioStream(stream1);
            SDL_PutAudioStreamData(stream1, wav_data2, wav_data_len2);
        }
        if (event->key.key == SDLK_LEFT && ((teto.x - 100) >= 20)){
            texture = texture2;
            teto.x -= 100; teto.dir = "left";
            SDL_ClearAudioStream(stream1);
            SDL_PutAudioStreamData(stream1, wav_data3, wav_data_len3);

        }
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
     /* see if we need to feed the audio stream more data yet.
       We're being lazy here, but if there's less than the entire wav file left to play,
       just shove a whole copy of it into the queue, so we always have _tons_ of
       data queued for playback. */
    // if (SDL_GetAudioStreamQueued(stream) < (int)wav_data_len) {
        /* feed more data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
        // SDL_PutAudioStreamData(stream, wav_data, wav_data_len);
    // }

    // const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */
    // /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    // const float red = (float) (0.5 + 0.5 * SDL_sin(now));
    // const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    // const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
    // SDL_SetRenderDrawColorFloat(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */
    SDL_FRect rect1, rect2 , rect3;

    // /* as you can see from this, rendering draws over whatever was drawn before it. */
    // SDL_SetRenderDrawColor(renderer, 33, 33, 33, SDL_ALPHA_OPAQUE);  /* dark gray, full alpha */

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
    rect1.x = rect1.y = 0;
    rect1.w = 640;
    rect1.h = 160;
    SDL_RenderFillRect(renderer, &rect1);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
    rect2.x = 0;
    rect2.y = 160;
    rect2.w = 640;
    rect2.h = 160;
    SDL_RenderFillRect(renderer, &rect2);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
    rect2.x = 0;
    rect2.y = 320;
    rect2.w = 640;
    rect2.h = 160;
    SDL_RenderFillRect(renderer, &rect2);

    SDL_FRect head, body, wing1, wing2, banner;

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
    body.x = 290; body.y = 210; body.w = 60; body.h = 60;
    SDL_RenderFillRect(renderer, &body);

    head.x = 315; head.y = 190; head.w = 20; head.h = 20;
    SDL_RenderFillRect(renderer, &head);

    SDL_SetRenderDrawColor(renderer, 230, 250, 0, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
    wing1.x = 260; wing1.y = 200; wing1.w = 30; wing1.h = 60;
    SDL_RenderFillRect(renderer, &wing1);

    wing2.x = 350; wing2.y = 200; wing2.w = 30; wing2.h = 60;
    SDL_RenderFillRect(renderer, &wing2);

    SDL_SetRenderDrawColor(renderer, 250, 240, 0, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
    banner.x = 280; banner.y = 270; banner.w = 80; banner.h = 30;
    SDL_RenderFillRect(renderer, &banner);

    const Uint64 now = SDL_GetTicks();
    const float elapsed = ((float) (now - last_time)) / 1000.0f;  /* seconds since last iteration */

    /* let's move all our points a little for a new frame. */
    for (int i = 0; i < NUM_POINTS; i++) {
        const float distance = elapsed * point_speeds_ptr[i];
        points_ptr[i].x += distance;
        points_ptr[i].y += distance;
        if ((points_ptr[i].x >= WINDOW_WIDTH) || (points_ptr[i].y >= WINDOW_HEIGHT)) {
            /* off the screen; restart it elsewhere! */
            if (SDL_rand(2)) {
                points_ptr[i].x = SDL_randf() * ((float) WINDOW_WIDTH);
                points_ptr[i].y = 0.0f;
            } else {
                points_ptr[i].x = 0.0f;
                points_ptr[i].y = SDL_randf() * ((float) WINDOW_HEIGHT);
            }
            point_speeds_ptr[i] = MIN_PIXELS_PER_SECOND + (SDL_randf() * (MAX_PIXELS_PER_SECOND - MIN_PIXELS_PER_SECOND));
        }
    }

    last_time = now;

    /* as you can see from this, rendering draws over whatever was drawn before it. */
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* black, full alpha */
    // SDL_RenderClear(renderer);  /* start with a blank canvas. */
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);  /* white, full alpha */
    SDL_RenderPoints(renderer, points_ptr, NUM_POINTS);  /* draw all the points! */

    SDL_FRect dst_rect;
    // const Uint64 now = SDL_GetTicks();

    /* we'll have some textures move around over a few seconds. */
    // SDL_Log("%ld", now);
    const float direction = ((now % 10000) > 5000) ? 1.0f : -1.0f;
    // SDL_Log("%f", direction);
    const float scale = ((float) (((int) (now % 10000)) - 5000) / 5000.0f) * direction;
    // SDL_Log("%f", scale);
    /* as you can see from this, rendering draws over whatever was drawn before it. */
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* black, full alpha */
    // SDL_RenderClear(renderer);  /* start with a blank canvas. */

    /* Just draw the static texture a few times. You can think of it like a
       stamp, there isn't a limit to the number of times you can draw with it. */

    // /* top left */
    const float rotation = (((float) ((int) (now % 5000))) / 5000.0f) * 360.0f;


    SDL_FPoint center;

    dst_rect.x = ((WINDOW_WIDTH - texture_width) * scale);
    dst_rect.y = ((WINDOW_HEIGHT - texture_height) * scale);
    dst_rect.w = (float) texture_width;
    dst_rect.h = (float) texture_height;


    center.x = (texture_width) / 2.0f;
    center.y = (texture_height) / 2.0f;
    
    SDL_SetTextureColorModFloat(texture, 0.0f, 0.0f, 1.0f);
    SDL_RenderTextureRotated(renderer, texture, NULL, &dst_rect, rotation, &center, SDL_FLIP_NONE);

    dst_rect.x = ( scale * (WINDOW_WIDTH - texture_width));
    dst_rect.y = ((WINDOW_HEIGHT - texture_height)- scale * (WINDOW_HEIGHT - texture_height));
    dst_rect.w = (float) texture_width;
    dst_rect.h = (float) texture_height;
    SDL_SetTextureColorModFloat(texture, 0.0f, 1.0f, 0.0f);
    SDL_RenderTextureRotated(renderer, texture, NULL, &dst_rect, rotation, &center, SDL_FLIP_NONE);

    dst_rect.x = ((WINDOW_WIDTH - texture_width) - scale * (WINDOW_WIDTH - texture_width));
    dst_rect.y = (scale * (WINDOW_HEIGHT - texture_height));
    dst_rect.w = (float) texture_width;
    dst_rect.h = (float) texture_height;
    SDL_SetTextureColorModFloat(texture, 1.0f, 1.0f, 1.0f);
    SDL_RenderTextureRotated(renderer, texture, NULL, &dst_rect, rotation, &center, SDL_FLIP_NONE);

    dst_rect.x = ( (WINDOW_WIDTH - texture_width) - scale * (WINDOW_WIDTH - texture_width));
    dst_rect.y = ((WINDOW_HEIGHT - texture_height)- scale * (WINDOW_HEIGHT - texture_height));
    dst_rect.w = (float) texture_width;
    dst_rect.h = (float) texture_height;
    SDL_SetTextureColorModFloat(texture, 1.0f, 1.0f, 0.0f);    
    SDL_RenderTexture(renderer, texture, NULL, &dst_rect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
    SDL_SetRenderScale(renderer, 2.0f, 2.0f);
    SDL_RenderDebugText(renderer, 0, 0, "Hello world!");
    SDL_RenderDebugTextFormat(renderer, 0, 10, "teto is %s :)", teto.dir);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    
    teto.teto_rect.x = 0;
    teto.teto_rect.y = 0;
    teto.teto_rect.w = 50;
    teto.teto_rect.h = 50;
    SDL_SetTextureColorModFloat(texture, 1.0f, 1.0f, 1.0f);    
    SDL_RenderTexture(renderer, texture, NULL, &teto.teto_rect);


    /* center this one. */
    // dst_rect.x = ((float) (WINDOW_WIDTH - texture_width)) / 2.0f;
    // dst_rect.y = ((float) (WINDOW_HEIGHT - texture_height)) / 2.0f;
    // dst_rect.w = (float) texture_width;
    // dst_rect.h = (float) texture_height;
    // SDL_RenderTexture(renderer, texture, NULL, &dst_rect);

    /* bottom right. */
    // dst_rect.x = ((float) (WINDOW_WIDTH - texture_width)) - (100.0f * scale);
    // dst_rect.y = (float) (WINDOW_HEIGHT - texture_height);
    // dst_rect.w = (float) texture_width;
    // dst_rect.h = (float) texture_height;
    // SDL_RenderTexture(renderer, texture, NULL, &dst_rect);

    // SDL_RenderPresent(renderer);  /* put it all on the screen! */

    /* You can also draw single points with SDL_RenderPoint(), but it's
       cheaper (sometimes significantly so) to do them all at once. */


    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
    // rect3.x = rect3.y = 320;
    // rect3.w = 640;
    // rect3.h = 160;
    // SDL_RenderFillRect(renderer, &rect3);

    // // SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);  /* red, full alpha */

    // // SDL_RenderClear(renderer);  /* start with a blank canvas. */

    // /* clear the window to the draw color. */
    // // SDL_RenderClear(renderer);

    // const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */
    // /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    // const float red = (float) (0.5 + 0.5 * SDL_sin(now));
    // const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    // const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
    // SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE);  /* new color, full alpha. */
    // SDL_RenderPoints(renderer, points, SDL_arraysize(points));

    // /* draw two lines in an X across the whole canvas. */
    // SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);  /* yellow, full alpha */
    // SDL_RenderLine(renderer, 0, 30, 640, 480);
    // SDL_RenderLine(renderer, 10, 20, 650, 470);
    // SDL_RenderLine(renderer, 20, 10, 660, 460);
    // SDL_RenderLine(renderer, 30, 0, 670, 450);
    // SDL_RenderLine(renderer, 0, 480, 640, 0);

    /* put the newly-cleared rendering on the screen. */
    SDL_RenderPresent(renderer);


    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_SDL_free(wav_data);  /* strictly speaking, this isn't necessary because the process is ending, but it's good policy. */
    SDL_SDL_free(wav_data1);  /* strictly speaking, this isn't necessary because the process is ending, but it's good policy. */
    SDL_SDL_free(wav_data2);  /* strictly speaking, this isn't necessary because the process is ending, but it's good policy. */
    SDL_SDL_free(wav_data3);  /* strictly speaking, this isn't necessary because the process is ending, but it's good policy. */
    if(points_ptr){
        SDL_free(points_ptr);
        points_ptr = NULL;
    }
    if(point_speeds_ptr){
        SDL_free(point_speeds_ptr);
        point_speeds_ptr = NULL;
    }
    window = NULL;
    renderer = NULL;

    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(texture1);
    SDL_DestroyTexture(texture2);
    /* SDL will clean up the window/renderer for us. */
}


 