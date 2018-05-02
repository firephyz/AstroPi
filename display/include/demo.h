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
  SDL_mutex * input_mutex;
  int has_new_data;
  int scroll;
  int button1;
  int button2;
  double joystick_x;
  double joystick_y;
} EventPacket;

typedef struct {
  SDL_mutex * display_ready_mutex;
  SDL_cond * display_ready_cond;
  DemoWindow * window;
} RenderThreadData;

typedef struct {
  DemoWindow * window;
  EventPacket * input_events;
} ComputeThreadData;

typedef struct {
  SDL_Thread * main_thread;
  SDL_Thread * compute_thread;
  EventPacket * input_events;
} EventThreadData;

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
