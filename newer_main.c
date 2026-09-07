// Velocity application

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800
#define MOTION_EVENT_COOLDOWN 40

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Joystick *joystick = NULL;

typedef enum { FACE_RIGHT, FACE_LEFT, FACE_UP, FACE_DOWN } Face_t;

typedef struct{
    // float v; // Velocity
    float a; // accelration
    float vi; // velocity initial
    Uint64 it; // initial time
    Uint64 lt; // last time
    float td; // total distance
    float pd; // passed distance
    float tp; // target position
    Face_t dir; // direction
    bool moving; // does the player moves?
} Moving_t;

typedef struct {
    SDL_FRect body;
    SDL_Texture *texture;
    Face_t face_position;
    Moving_t move;
} Player_t;

static Player_t player;

float calc_moving_distance(){
    Uint64 now = SDL_GetTicks();
    Uint64 delta_t = now - player.move.lt;
    player.move.lt = now;
    float velocity = player.move.vi + player.move.a * (now - player.move.it);
    float distance = (float)delta_t * velocity;
    // SDL_Log("getticks %f velocity %f delta distance %f vi %f a %f", (float)now, velocity, distance, player.move.vi ,player.move.a);
    return distance;
}

void start_moving(float dist, /* float v ,*/ Face_t dir, float tp, float vi, float vf){
    if(!player.move.moving){
        // player.move.v = v; 
        player.move.lt = SDL_GetTicks();
        player.move.td = dist;
        player.move.pd = 0.0f;
        player.move.tp = tp;
        player.move.moving = true;
        player.move.dir = dir;
        player.move.vi = vi;
        player.move.a = ((vf*vf) - (vi*vi)) / ( 2 * dist);
        player.move.it = SDL_GetTicks();
    }
}

void stop_moving(){
    // player.move.v = 0.0f; 
    player.move.lt = 0;
    player.move.td = 0.0f;
    player.move.pd = 0.0f;
    player.move.a = 0;
    player.move.vi = 0;
    // player.move.vf = 0;
    player.move.moving = false;

    switch(player.move.dir){
    case FACE_UP:
    case FACE_DOWN:
        player.body.y = player.move.tp;
        break;
    case FACE_RIGHT:
    case FACE_LEFT:
        player.body.x = player.move.tp;
        break;
    }

}

void do_move(){
    float dist = calc_moving_distance();
    // SDL_Log("acc %f vi %f d %f passed d %f total d %f", player.move.a, player.move.vi, dist, player.move.pd, player.move.td);
    if(
        player.move.td < (player.move.pd + dist) || dist < 0
        // || ((player.move.pd + dist) > WINDOW_HEIGHT && ((player.move.dir == FACE_UP) || (player.move.dir == FACE_DOWN)) ) 
        // || ((player.move.pd + dist) > WINDOW_WIDTH && ((player.move.dir == FACE_RIGHT) || (player.move.dir == FACE_LEFT)) ) 
    ){
        // dist = player.move.td - player.move.pd;
        stop_moving();
    }else{
        player.move.pd += dist;
        switch(player.move.dir){
            case FACE_UP:
                ((player.body.y - dist) > 0) ? player.body.y -= dist : stop_moving();
                break;
            case FACE_DOWN:
                ((player.body.y + dist) < WINDOW_HEIGHT) ? player.body.y += dist : stop_moving();
                break;
            case FACE_RIGHT:
                ((player.body.x + dist) < WINDOW_WIDTH) ? player.body.x += dist : stop_moving();
                // player.body.x += dist;
                break;
            case FACE_LEFT:
                ((player.body.x - dist) > 0) ? player.body.x -= dist : stop_moving();
                // player.body.x -= dist;
                break;
        }
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Velocity", "1.0", "com.example.idk");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Velocity", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_Surface *surface = NULL;
    char *png_path = NULL;

    /* Textures are pixel data that we upload to the video hardware for fast drawing. Lots of 2D
       engines refer to these as "sprites." We'll do a static texture (upload once, draw many
       times) with data from a png file. */

    /* SDL_Surface is pixel data the CPU can access. SDL_Texture is pixel data the GPU can access.
       Load a .png into a surface, move it to a texture from there. */
    SDL_asprintf(&png_path, "%steto.png", SDL_GetBasePath());  /* allocate a string of the full file path */
    surface = SDL_LoadPNG(png_path);
    SDL_SDL_free(png_path);  /* done with this, the file is loaded. */

    if (!surface) {
        SDL_Log("Couldn't load png: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    player.body.x = 0;
    player.body.y = 0;
    player.body.w = 100;
    player.body.h = 100;
    player.move.lt = 0;

    player.texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);  /* done with this, the texture has a copy of the pixels now. */

    if (!player.texture) {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    if (event->type == SDL_EVENT_JOYSTICK_ADDED) {
        /* this event is sent for each hotplugged stick, but also each already-connected joystick during SDL_Init(). */
        const SDL_JoystickID which = event->jdevice.which;
        joystick = SDL_OpenJoystick(which);
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
        SDL_Log( "Joystick #%u removed", (unsigned int) which);
    } 

    // if (event->type == SDL_EVENT_KEY_DOWN) {
    //     // move up, going to y = 0
    // }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

void IterateInput(){
    // move up, get close to y = 0
    if (SDL_GetJoystickAxis(joystick, 1) < -24543 && ((player.body.y - 100) >=  0)){
        start_moving(100, FACE_UP, player.body.y - 100, 0.9, 0.9);
        player.face_position = FACE_UP ;
    }

    // move down, get far from y = 0
    if (SDL_GetJoystickAxis(joystick, 1) > 24543 && ((player.body.y + 100) <= WINDOW_HEIGHT - player.body.h ) )
    {
        start_moving(500, FACE_DOWN, player.body.y + 500, 0.1, 1);
        player.face_position = FACE_DOWN;
    }

    // move right, get far from x = 0
    if (SDL_GetJoystickAxis(joystick, 0) > 24543 && ((player.body.x + 100) <= WINDOW_WIDTH - player.body.w )){
        start_moving(600, FACE_RIGHT, player.body.x + 600, 1, 0);
        player.face_position = FACE_RIGHT ;
    }

    // move left, get close to x = 0
    if (SDL_GetJoystickAxis(joystick, 0) < -24543 && ((player.body.x - 100) >= 0 )){
        start_moving(100, FACE_LEFT, player.body.x - 100, 0.9, 0.9);
        player.face_position = FACE_LEFT ;
    }

}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    IterateInput();

    SDL_SetRenderDrawColor(renderer, 40, 40, 40, SDL_ALPHA_OPAQUE); // make a black-gray background
    SDL_RenderClear(renderer); // clear the canvas

    if(player.move.moving){ // update frames by moving
        do_move();
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); // change to white color
    for(int i = 0; i < (WINDOW_WIDTH/player.body.w); i++){
        SDL_RenderLine(renderer, i*100, 0, i*100, WINDOW_HEIGHT);
        SDL_RenderLine(renderer, 0, i*100, WINDOW_WIDTH, i*100);
    }

    SDL_RenderTexture(renderer, player.texture, NULL, &player.body);

    /* put the newly-cleared rendering on the screen. */
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(player.texture);
    if (joystick) {
        SDL_CloseJoystick(joystick);  /* the joystick was unplugged. */
    }
    window = NULL;
    renderer = NULL;

    /* SDL will clean up the window/renderer for us. */
}
