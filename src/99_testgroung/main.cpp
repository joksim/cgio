#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cmath>
#include <utility>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;
static Uint64 last_time = 0;

const int WIN_WIDTH = 600;
const int WIN_HEIGHT = 600;

int lines[10][4];

void line(int ax, int ay, int bx, int by) {
    for (float t = 0.; t < 1.; t += .02) {
        int x = std::round(ax + (bx - ax) * t);
        int y = std::round(ay + (by - ay) * t);
        SDL_RenderPoint(renderer, x, y);
    }
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("CGIO-SDL3", "1.0", "com.cg-io.sdl3");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("SDL3 Windows", WIN_WIDTH, WIN_HEIGHT,
                                     SDL_WINDOW_RESIZABLE, &window,
                                     &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WIN_WIDTH, WIN_HEIGHT,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    for (int i = 0; i < 10; i++) {
        lines[i][0] = rand() % WIN_WIDTH;
        lines[i][1] = rand() % WIN_HEIGHT;
        lines[i][2] = rand() % WIN_WIDTH;
        lines[i][3] = rand() % WIN_HEIGHT;
    }

    last_time = SDL_GetTicks();

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS.
                                 */
    }
    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {
    const Uint64 now = SDL_GetTicks();
    const float elapsed =
        ((float)(now - last_time)) / 1000.0f; /* seconds since last iteration */

    last_time = now;

    /* as you can see from this, rendering draws over whatever was drawn before
     * it. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0,
                           SDL_ALPHA_OPAQUE); /* black, full alpha */
    SDL_RenderClear(renderer);                /* start with a blank canvas. */

    SDL_SetRenderDrawColor(renderer, 255, 255, 255,
                           SDL_ALPHA_OPAQUE); /* white, full alpha */

    for (auto i = 0; i < 10; i++) {
        line(lines[i][0], lines[i][1], lines[i][2], lines[i][3]);
    }

    SDL_RenderPresent(renderer); /* put it all on the screen! */

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    /* SDL will clean up the window/renderer for us. */
}
