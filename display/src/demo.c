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

  RenderThreadData render_data = {SDL_CreateMutex(),
				SDL_CreateCond(),
				NULL};
  SDL_Thread * render_thread = SDL_CreateThread(launch_render_thread,
						"Render Thread",
						&render_data);
  
  // Wait for main thread to finish creating
  // the display before we continue
  SDL_LockMutex(render_data.display_ready_mutex);
  SDL_CondWait(render_data.display_ready_cond,
	       render_data.display_ready_mutex);
  SDL_UnlockMutex(render_data.display_ready_mutex);
  

  // Create the primary rendering and computation thread
  ComputeThreadData compute_data = {render_data.window};
  EventThreadData event_data = {render_thread,
				SDL_CreateThread(launch_compute_thread,
						 "Compute Thread",
						 &compute_data)};

  
  // Let this thread continue running the event handler
  handle_events(&event_data);

  SDL_DestroyCond(render_data.display_ready_cond);
  SDL_DestroyMutex(render_data.display_ready_mutex);
  destroy_demo_window(render_data.window);
  return 0;
}

int launch_compute_thread(void * void_data) {

  ComputeThreadData * data = (ComputeThreadData *)void_data;

  double width = 7.5;
  double height = 5;
  double delta = 0.000001;
  GravityBody sun = {{-2, 0},
		     {0, 0.05},
		     100};
  GravityBody planet = {{-0.3, 0},
			{0, -1},
			5};
  while(1) {
    CHECK_QUIT_REQUESTED;

    // Calculate new position
    Vector new_sun_pos = {sun.pos.x + sun.vel.x * delta,
			  sun.pos.y + sun.vel.y * delta};
    Vector new_planet_pos = {planet.pos.x + planet.vel.x * delta,
			     planet.pos.y + planet.vel.y * delta};

    // Calculate new velocity
    double dx = sun.pos.x - planet.pos.x;
    double dy = sun.pos.y - planet.pos.y;
    double distance = sqrt(dx * dx + dy * dy);
    double force = 0.05 * sun.mass * planet.mass / distance / distance;

    Vector sun_acc = {-force * dx / distance / sun.mass,
		      -force * dy / distance / sun.mass};
    Vector planet_acc = {force * dx / distance / planet.mass,
			 force * dy / distance / planet.mass};
    Vector new_sun_vel = {sun.vel.x + sun_acc.x * delta,
			  sun.vel.y + sun_acc.y * delta};
    Vector new_planet_vel = {planet.vel.x + planet_acc.x * delta,
			     planet.vel.y + planet_acc.y * delta};

    sun.pos = new_sun_pos;
    sun.vel = new_sun_vel;
    planet.pos = new_planet_pos;
    planet.vel = new_planet_vel;

    int sun_x = (width / 2 + sun.pos.x) / width * data->window->width;
    int sun_y = (height / 2 - sun.pos.y) / height * data->window->height;
    int planet_x = (width / 2 + planet.pos.x) / width * data->window->width;
    int planet_y = (height / 2 - planet.pos.y) / height * data->window->height;

    SDL_LockMutex(data->window->buffer_mutex);
    data->window->draw_buffer[sun_x + sun_y * data->window->width] = 1;
    data->window->draw_buffer[planet_x + planet_y * data->window->width] = 1;
    SDL_UnlockMutex(data->window->buffer_mutex);

    struct timespec delay = {0, 1000};
    //    nanosleep(&delay, NULL);
  }

  return 0;
}

int launch_render_thread(void * void_data) {

  RenderThreadData * data = (RenderThreadData *)void_data;
  struct timespec frame_delay_time = {0, 1000000000/30};
  DemoWindow * window = create_demo_window(960, 640);
  SDL_Renderer * renderer = window->renderer;

  // Notify event thread that the display is created
  data->window = window;
  SDL_LockMutex(data->display_ready_mutex);
  SDL_CondBroadcast(data->display_ready_cond);
  SDL_UnlockMutex(data->display_ready_mutex);  

  while(1) {
    CHECK_QUIT_REQUESTED;

    // Switch double buffers
    SDL_LockMutex(window->buffer_mutex);
    for(int i = 0; i < window->height * window->width; ++i ) {
	window->present_buffer[i] = window->draw_buffer[i];
    }
    int * temp_buffer = window->draw_buffer;
    window->draw_buffer = window->present_buffer;
    window->present_buffer = temp_buffer;
    SDL_UnlockMutex(window->buffer_mutex);  

    // Draw the new data
    SDL_RenderClear(renderer);
    for(int y = 0; y < window->height; ++y) {
      for(int x = 0; x < window->width; ++x) {
	if(window->present_buffer[x + y * window->width] == 1) {
	  SDL_Point points[3] = {{2 * x    , 2 * y    },
				 {2 * x + 1, 2 * y    },
				 {2 * x    , 2 * y + 1}};
	  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	  SDL_RenderDrawPoints(renderer, points, 3);
	}
	else {
	  SDL_Rect rect = {2 * x, 2 * y};
	  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
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
  
  while(1) {
    CHECK_QUIT_REQUESTED;
    
    if(SDL_WaitEvent(&event)) {
      switch(event.type) {
      case SDL_QUIT:
	SDL_LockMutex(mutex_is_running);
	is_running = 0;
	SDL_UnlockMutex(mutex_is_running);
	break;
      case SDL_MOUSEBUTTONDOWN:;
	break;
      case SDL_MOUSEMOTION:
	break;
      }
    }
  }

  SDL_WaitThread(data->main_thread, NULL);
  SDL_WaitThread(data->compute_thread, NULL);
}

DemoWindow * create_demo_window(int width, int height) {

  DemoWindow * window = malloc(sizeof(DemoWindow));
  *window = (DemoWindow){width / 2,
			 height / 2,
			 NULL,
			 NULL,
			 SDL_CreateMutex(),
			 calloc(width * height / 4, sizeof(int)),
			 calloc(width * height / 4, sizeof(int))};
  
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

  SDL_SetRenderDrawColor(window->renderer, 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderClear(window->renderer);
  SDL_RenderPresent(window->renderer);

  return window;
}

void destroy_demo_window(DemoWindow * window) {

  SDL_DestroyRenderer(window->renderer);
  SDL_DestroyWindow(window->window);
  free(window);
}
