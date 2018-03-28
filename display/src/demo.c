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
  
  DemoWindow * window = NULL;
  SDL_mutex * display_read_mutex = SDL_CreateMutex();
  SDL_cond * display_read_cond = SDL_CreateCond();
  MainThreadData main_data = {display_read_mutex,
			      display_read_cond,
			      &window};
  EventThreadData event_data = {display_read_mutex,
				display_read_cond,
				&window,
				NULL};

  // Create the primary rendering and computation thread
  event_data.main_thread = SDL_CreateThread(launch_main_thread,
					    "Primary working thread",
					    &main_data);
  // Let this thread continue running the event handler
  handle_events(&event_data);

  SDL_DestroyCond(display_read_cond);
  SDL_DestroyMutex(display_read_mutex);
  destroy_demo_window(window);
  return 0;
}

int launch_main_thread(void * void_data) {

  MainThreadData * data = (MainThreadData *)void_data;
  struct timespec frame_delay_time = {0, 50000000};
  DemoWindow * window = create_demo_window(50, 50);
  SDL_Renderer * renderer = window->renderer;

  // Notify event thread that the display is created
  *((MainThreadData *)void_data)->window = window;
  SDL_LockMutex(data->display_read_mutex);
  SDL_CondSignal(data->display_read_cond);
  SDL_UnlockMutex(data->display_read_mutex);  

  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  while(1) {
    SDL_LockMutex(mutex_is_running);
    if(!is_running) break;
    SDL_UnlockMutex(mutex_is_running);

    // Switch double buffers
    SDL_LockMutex(window->buffer_mutex);
    // Update double buffers
    for(int y = 0; y < window->height; ++y) {
      for(int x = 0; x < window->width; ++x) {
	window->present_buffer[x + y * window->width] = window->draw_buffer[x + y * window->width];
      }
    }
    int * temp_buffer = window->draw_buffer;
    window->draw_buffer = window->present_buffer;
    window->present_buffer = temp_buffer;
    SDL_UnlockMutex(window->buffer_mutex);  

    // Draw the new data
    SDL_Point points[3] = {{0, 0},
			   {1, 0},
			   {0, 1}};
    SDL_Rect rect = {0, 0, 2, 2};
    for(int y = 0; y < window->height / 2; ++y) {
      for(int x = 0; x < window->width / 2; ++x) {
	if(window->present_buffer[(2 * x) + (2 * y) * window->width] == 1) {
	  points[0] = (SDL_Point){2 * x, 2 * y};
	  points[1] = (SDL_Point){2 * x + 1, 2 * y};
	  points[2] = (SDL_Point){2 * x, 2 * y + 1};
	  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0x00);
	  SDL_RenderDrawPoints(renderer, points, 3);
	}
	else {
	  rect.x = 2 * x;
	  rect.y = 2 * y;
	  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
	  SDL_RenderFillRect(renderer, &rect);
	}
      }
    }
    
    SDL_RenderPresent(renderer);
    nanosleep(&frame_delay_time, NULL);
  }

  return 0;
}

void handle_events(EventThreadData * data) {

  SDL_Event event;
  struct timespec event_delay_time = {0, 50000000};
  int pixel_value;

  // Wait for main thread to create the display
  SDL_LockMutex(data->display_read_mutex);
  SDL_CondWait(data->display_read_cond, data->display_read_mutex);
  SDL_UnlockMutex(data->display_read_mutex);  
  DemoWindow * window = *data->window;
  
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
      case SDL_MOUSEBUTTONDOWN:;
	int x = event.button.x;
	int y = event.button.y;
	SDL_LockMutex(window->buffer_mutex);
	int index = ((x / 2) * 2) + ((y / 2) * 2) * window->width;
	pixel_value = window->draw_buffer[index] == 1 ? 0 : 1;
	window->draw_buffer[index] = pixel_value;
	SDL_UnlockMutex(window->buffer_mutex);
	break;
      case SDL_MOUSEMOTION:
	if(event.motion.state & SDL_BUTTON_LMASK) {
	  int x = event.motion.x;
	  int y = event.motion.y;
	  SDL_LockMutex(window->buffer_mutex);
	  int index = ((x / 2) * 2) + ((y / 2) * 2) * window->width;
	  window->draw_buffer[index] = pixel_value;
	  SDL_UnlockMutex(window->buffer_mutex);	  
	}
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
