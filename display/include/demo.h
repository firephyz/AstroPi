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
  SDL_mutex * display_read_mutex;
  SDL_cond * display_read_cond;
  DemoWindow ** window;
} MainThreadData;

typedef struct {
  SDL_mutex * display_read_mutex;
  SDL_cond * display_read_cond;
  DemoWindow ** window;
  SDL_Thread * main_thread;
} EventThreadData;

DemoWindow * create_demo_window(int width, int height);
void destroy_demo_window(DemoWindow * window);

int launch_main_thread(void * void_data);
void handle_events(EventThreadData * data);

#endif
