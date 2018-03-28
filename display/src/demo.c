#define _POSIX_C_SOURCE 199506L

#include "demo.h"
#include "SDL.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

SDL_mutex * mutex_is_running;
int is_running = 1;

int main(int argc, char **argv) {

  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Failed to init SDL: %s\n", SDL_GetError());
    exit(-1);
  }

  mutex_is_running = SDL_CreateMutex();
  if(mutex_is_running == NULL) {
    fprintf(stderr, "Error creating run mutex:%s\n", SDL_GetError());
    exit(-1);
  }
  
  DemoWindow * window = create_demo_window(480, 320);
  MainThreadData main_data = {window};

  // Create main rendering and computation thread
  SDL_Thread * main_thread = SDL_CreateThread(launch_main_thread,
					      "Primary working thread",
					      &main_data);
  EventThreadData event_data = {window,
				main_thread};
  handle_events(&event_data);
  
  destroy_demo_window(window);
  return 0;
}

int launch_main_thread(void * void_data) {

  MainThreadData * data = (MainThreadData *)void_data;
  SDL_Renderer * renderer = data->window->renderer;
  struct timespec frame_delay_time = {0, 50000000};

  SDL_Rect rect = {0, 0, 10, 10};
  while(1) {
    SDL_LockMutex(mutex_is_running);
    if(!is_running) break;
    SDL_UnlockMutex(mutex_is_running);

    SDL_LockMutex(data->window->render_mutex);
    SDL_RenderPresent(renderer);
    SDL_UnlockMutex(data->window->render_mutex);
    nanosleep(&frame_delay_time, NULL);
  }

  return 0;
}

void handle_events(EventThreadData * data) {

  SDL_Event event;
  struct timespec event_delay_time = {0, 50000000};
  
  while(1) {

    SDL_LockMutex(mutex_is_running);
    if(!is_running) break;
    SDL_UnlockMutex(mutex_is_running);
    
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT:
	SDL_LockMutex(mutex_is_running);
	is_running = 0;
	SDL_UnlockMutex(mutex_is_running);	is_running = 0;
	goto event_thread_exit;
	break;
      case SDL_MOUSEBUTTONDOWN:
	int x = event.button.x / 2;
	int y = event.button.y / 2;
	SDL_LockMutex(data->window->render_mutex);
	data->window->buffer[x + y * 160] ^= 1;
	SDL_UnlockMutex(data->window->render_mutex);
	break;
      }
    }
    
    nanosleep(&event_delay_time, NULL);
  }

 event_thread_exit:
  SDL_WaitThread(data->main_thread, NULL);
}

DemoWindow * create_demo_window(int width, int height) {

  DemoWindow * window = malloc(sizeof(DemoWindow));
  *window = (DemoWindow){width,
			 height,
			 NULL,
			 NULL,
			 SDL_CreateMutex(),
			 calloc(width * height, sizeof(int)),
			 calloc(width * height, sizeof(int))};
  
  if(SDL_CreateWindowAndRenderer(width,
				 height,
				 0x0,
				 &window->window,
				 &window->renderer) != 0) {
    fprintf(stderr,
	    "Error creating window and renderer: %s\n",
	    SDL_GetError());
    exit(-1);
  }

  return window;
}

void destroy_demo_window(DemoWindow * window) {

  SDL_DestroyRenderer(window->renderer);
  SDL_DestroyWindow(window->window);
  free(window);
}
