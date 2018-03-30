#ifndef DEMO_INCLUDED

#define DEMO_INCLUDED

#include "SDL.h"

typedef struct {
  int width;
  int height;
  SDL_Window * window;
  SDL_Renderer * renderer;
  SDL_mutex * buffer_mutex;
  int *draw_buffer;
  int *present_buffer;
} DemoWindow;

typedef struct {
  SDL_mutex * display_ready_mutex;
  SDL_cond * display_ready_cond;
  DemoWindow * window;
} RenderThreadData;

typedef struct {
  DemoWindow * window;
} ComputeThreadData;

typedef struct {
  SDL_Thread * main_thread;
  SDL_Thread * compute_thread;
} EventThreadData;

typedef struct {
  double x;
  double y;
} Vector;

typedef struct {
  Vector pos;
  Vector vel;
  double mass;
} GravityBody;

#define CHECK_QUIT_REQUESTED\
  SDL_LockMutex(mutex_is_running);\
  int quit_requested = !is_running;\
  SDL_UnlockMutex(mutex_is_running);\
  if(quit_requested) break;

DemoWindow * create_demo_window(int width, int height);
void destroy_demo_window(DemoWindow * window);

int launch_render_thread(void * void_data);
int launch_compute_thread(void * void_data);
void handle_events(EventThreadData * data);

#endif
