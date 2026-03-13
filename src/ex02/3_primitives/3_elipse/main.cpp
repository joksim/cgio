#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <cmath>
#include <ctime>
#include <exception>
#include <vector>

struct Vertex2i {
  int x;
  int y;
  Vertex2i(int _x, int _y):x(_x),y(_y){}
};


std::vector<Vertex2i> points;

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;
static Uint64 last_time = 0;

int WIN_WIDTH = 600;
int WIN_HEIGHT = 600;


void gen_elipse(const Vertex2i& center, int rx, int ry, int half_segments = 32 ) {
  double angle = 0.0;
  double angle_increment = M_PI / half_segments;

  for (auto i=0; i<=2*half_segments; i++) {
     points.emplace_back(center.x + rx*cos(angle),center.y + ry*sin(angle));
    angle+=angle_increment;
  }
}




void line(Vertex2i& a, Vertex2i&b) {
  int ax, ay, bx, by;
  ax = a.x;
  ay = a.y;
  bx = b.x;
  by = b.y;
  bool steep = std::abs(ax-bx) < std::abs(ay-by);
  if (steep) { // if the line is steep, we transpose the image
    std::swap(ax, ay);
    std::swap(bx, by);
  }
  if (ax>bx) { // make it left−to−right
    std::swap(ax, bx);
    std::swap(ay, by);
  }
  for (int x=ax; x<=bx; x++) {
    float t = (x-ax) / static_cast<float>(bx-ax);
    int y = std::round( ay + (by-ay)*t );
    if (steep) // if transposed, de−transpose
      SDL_RenderPoint(renderer, y, x);
    else
      SDL_RenderPoint(renderer, x, y);
  }
}


void line_loop(std::vector<Vertex2i>& vertices) {
  for (int i=0; i<vertices.size()-1; i++) {
    line(vertices[i], vertices[i+1]);
  }
  //line(vertices[vertices.size()-1],vertices[0]);
}


/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {

  // Set metadata about the program
  SDL_SetAppMetadata("CGIO-SDL3", "1.0", "com.cg-io.sdl3");

  // Initialize SDL Video subsystem
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create the main window with the specific properties
  if (!SDL_CreateWindowAndRenderer("CG-IO: SDL3 Basic window creation example",
                                   WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_RESIZABLE,
                                   &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Set the presentation and resolution mode
  SDL_SetRenderLogicalPresentation(renderer, WIN_WIDTH, WIN_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  // Load and set the Window icon
  SDL_Surface *icon_surf = SDL_LoadBMP("res/io.bmp");
  if (!icon_surf) {
    SDL_Log("Error creating icon Surface: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  if (!SDL_SetWindowIcon(window, icon_surf)) {
    SDL_Log("Error setting Window Icon: %s", SDL_GetError());
    SDL_DestroySurface(icon_surf);
    return SDL_APP_FAILURE;
  }

  // Destroy the icon surface
  SDL_DestroySurface(icon_surf);

  gen_elipse(Vertex2i(WIN_WIDTH/2,WIN_HEIGHT/2),WIN_WIDTH/3, WIN_HEIGHT/4, 50);

  // Record the time before the render loop starts
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


  line_loop(points);


  SDL_RenderPresent(renderer); /* put it all on the screen! */

  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  /* SDL will clean up the window/renderer for us. */
}
