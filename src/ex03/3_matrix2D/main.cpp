#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <climits>
#include <cmath>
#include <ctime>
#include <vector>
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/* We need two types of vertices */

// int  for viewport coordinates

struct Vertex2i {
  int x;
  int y;
  Vertex2i(int _x, int _y) : x(_x), y(_y) {}
};

// Float vertices for working with homogenous coordinates
struct Vertex2f {
  float x;
  float y;
  float w; // added the homogenous coordinate
  Vertex2f(float _x, float _y, float _w = 1.f) : x(_x), y(_y), w(_w) {}
};

// This will currently serve as the main working points
std::vector<Vertex2f> points = {{.0f, -.5f}, {.5f, -.5f}, {0.f, .5f}};

std::vector<Vertex2f> drawpoints;

// Viewport points will be used when transforming the points into viewport
// coords
std::vector<Vertex2i> screenspace_points;

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;
static Uint64 last_time = 0;

int WIN_WIDTH = 600;
int WIN_HEIGHT = 600;

float MODEL_MATRIX[3][3];

void identity(float m[3][3]) {
  /* We can initialize the matrix using nested loops */
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (i == j) {
        m[i][j] = 1.f;
      } else {
        m[i][j] = 0.f;
      }
    }
  }
}

void apply_transform(float m[3][3]) {
  /* Create a temporary result matrix */
  float tmp[3][3]{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

  /* Multiply the MODEL_MATRIX with m */
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        tmp[i][j] += MODEL_MATRIX[i][k] * m[k][j];
      }
    }
  }

  /* Set the MODEL_MATRIX to the temporary result matrix */
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      MODEL_MATRIX[i][j] = tmp[i][j];
    }
  }
}

void translate(float tx, float ty) {
  /* Create the 2D 3x3 translation matrix */
  float rotate[3][3] = {{1.f, 0.f, 0.f},
                        {0.f, 1.f, 0.f},
                        {tx, tx, 1.f}};

  apply_transform(rotate);

}
void rotate(float angle) {
  /* Create the 2D 3x3 translation matrix */
  float rotate[3][3] = {{std::cos(angle), std::sin(angle), 0.f},
                        {-std::sin(angle), std::cos(angle), 0.f},
                        {0.f, 0.f, 1.f}};

  apply_transform(rotate);
}

void scale(float sx=1.f, float sy=1.f) {
  float scale[3][3] = {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {0.f, 0.f, 1.f}};

  apply_transform(scale);
}

void model_apply(std::vector<Vertex2f> &vertices) {
  drawpoints.clear();
  float dx, dy, dw;
  for (auto &v : vertices) {
    dx = v.x * MODEL_MATRIX[0][0] + v.y * MODEL_MATRIX[1][0] +
         v.w * MODEL_MATRIX[2][0];
    dy = v.x * MODEL_MATRIX[0][1] + v.y * MODEL_MATRIX[1][1] +
         v.w * MODEL_MATRIX[2][1];
    dw = v.x * MODEL_MATRIX[0][2] + v.y * MODEL_MATRIX[1][2] +
         v.w * MODEL_MATRIX[2][2];
    drawpoints.emplace_back(dx, dy, dw);
  }
}

// Function to map the points into the viewport coordinates
void screenspace_transform(const std::vector<Vertex2f> &points) {
  screenspace_points.clear();
  for (auto p : points) {
    float xs = 0 + ((p.x + 1) / 2) * WIN_WIDTH;
    float ys = WIN_HEIGHT - ((p.y + 1) / 2) * WIN_HEIGHT;
    screenspace_points.emplace_back(xs, ys);
  }
}

void line(const Vertex2i &a, const Vertex2i &b) {
  int ax = a.x;
  int ay = a.y;
  int bx = b.x;
  int by = b.y;
  bool steep = std::abs(ax - bx) < std::abs(ay - by);
  if (steep) { // if the line is steep, we transpose the image
    std::swap(ax, ay);
    std::swap(bx, by);
  }
  if (ax > bx) { // make it left−to−right
    std::swap(ax, bx);
    std::swap(ay, by);
  }
  for (int x = ax; x <= bx; x++) {
    float t = (x - ax) / static_cast<float>(bx - ax);
    int y = std::round(ay + (by - ay) * t);
    if (steep) // if transposed, de−transpose
      SDL_RenderPoint(renderer, y, x);
    else
      SDL_RenderPoint(renderer, x, y);
  }
}

void line_loop(const std::vector<Vertex2f> &vertices, uint start_index = 0,
               int num_points = -1) {
  if (num_points < 0)
    num_points = INT_MAX;
  num_points = MIN(vertices.size(), num_points);

  screenspace_transform(vertices);

  for (int i = 0; i < num_points - 1; i++) {
    line(screenspace_points[start_index + i],
         screenspace_points[start_index + i + 1]);
  }
  line(screenspace_points[start_index + num_points - 1],
       screenspace_points[start_index]);
}

void scene_setup() {
  // gen_regular_poly(Vertex2f(0, 0), 1.f, 3);
}

void draw() {
  drawpoints = points;
  identity(MODEL_MATRIX);
  translate(.5f, -.5f);
  rotate(M_PI_2);
  scale(1.f,.5f);
  model_apply(points);
  line_loop(drawpoints);
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

  // Create a single function for scene setup
  scene_setup();

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

  // Create a draw function
  draw();

  SDL_RenderPresent(renderer); /* put it all on the screen! */

  return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  /* SDL will clean up the window/renderer for us. */
}
