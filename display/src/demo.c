#define _POSIX_C_SOURCE 199506L

#include "demo.h"
#include "vector.h"
#include "planet_renderer.h"
#include "planet_database.h"
#include "fonts.h"
#include "SDL.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

SDL_mutex * mutex_is_running;
int is_running = 1;
Glyph * font_glyphs;

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

	initFontGlyphs();

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
	EventPacket input_events = {SDL_CreateMutex(), 0};
	ComputeThreadData compute_data = {render_data.window, &input_events};
	EventThreadData event_data = {
		render_thread,
		SDL_CreateThread(launch_compute_thread,
		                 "Compute Thread",
		                 &compute_data),
		&input_events
	};

	// Let this thread continue running the event handler
	handle_events(&event_data);

	SDL_DestroyCond(render_data.display_ready_cond);
	SDL_DestroyMutex(render_data.display_ready_mutex);
	destroy_demo_window(render_data.window);
	return 0;
}

int launch_compute_thread(void * void_data) {

	PlanetRenderer planets = {
		MAX_NUMBER_OF_PARENT_BODIES,
		malloc(sizeof(SolarBody) * MAX_NUMBER_OF_PARENT_BODIES),
		(TeleCamera){1, (Vector){0, 0, 0}}
	};
	for(int i = 0; i < MAX_NUMBER_OF_PARENT_BODIES; ++i) {
		planets.bodies[i] = (SolarBody){0, (Vector){0, 0, 0}, 0, NULL};
	}
	planets.number_of_bodies = 0;
	updateBodies(&planets);
	ComputeThreadData * data = (ComputeThreadData *)void_data;

	while(1) {
		CHECK_QUIT_REQUESTED;

		SDL_LockMutex(data->window->buffer_mutex);
		processPlanetEvents(&planets, data->input_events);
		renderPlanets(&planets, data->window);
		renderMenu(data->window);
		SDL_UnlockMutex(data->window->buffer_mutex);
	}

	return 0;
}

int launch_render_thread(void * void_data) {

	RenderThreadData * data = (RenderThreadData *)void_data;
	struct timespec frame_delay_time = {0, 1000000000/10};
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
				int buffer_index = x + y * window->width;
				if(window->present_buffer[buffer_index] == 1) {
					SDL_Point points[4] = {
						{2 * x,     2 * y    },
						{2 * x + 1, 2 * y    },
						{2 * x,     2 * y + 1},
						{2 * x + 1, 2 * y + 1}
					};
					SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderDrawPoints(renderer, points, 3);
					SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderDrawPoints(renderer, points + 3, 1);
				}
				else {
					SDL_Rect rect = {2 * x, 2 * y};
					SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderFillRect(renderer, &rect);
				}

				// Clear the present buffer for the next draw
				window->present_buffer[buffer_index] = 0;
			}
		}

		SDL_RenderPresent(renderer);
		nanosleep(&frame_delay_time, NULL);
	}

	return 0;
}

void handle_events(EventThreadData * data) {

	SDL_Event event;
	EventPacket * packet = data->input_events;

	int joystick_start_x = -1;
	int joystick_start_y = -1;
	int mouse_is_down = 0;

	while(1) {
		CHECK_QUIT_REQUESTED;

		if(SDL_WaitEvent(&event)) {
			switch(event.type) {
			case SDL_QUIT:
				SDL_LockMutex(mutex_is_running);
				is_running = 0;
				SDL_UnlockMutex(mutex_is_running);
				break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				SDL_LockMutex(packet->input_mutex);
				switch(event.key.keysym.sym) {
				case SDLK_UP:
					if(event.key.state == SDL_PRESSED) {
						packet->scroll = 1;
					}
					else {
						packet->scroll = 0;
					}
					break;
				case SDLK_DOWN:
					if(event.key.state == SDL_PRESSED) {
						packet->scroll = -1;
					}
					else {
						packet->scroll = 0;
					}
					break;
				case SDLK_LEFT:
					if(event.key.state == SDL_PRESSED) {
						packet->button1 = 1;
					}
					else {
						packet->button1 = 0;
					}
					break;
				case SDLK_RIGHT:
					if(event.key.state == SDL_PRESSED) {
						packet->button2 = 1;
					}
					else {
						packet->button2 = 0;
					}
					break;
				default:
					fprintf(stderr, "Invalid keypress.\n");
					break;
				}
				packet->has_new_data = 1;
				SDL_UnlockMutex(packet->input_mutex);
				break;
			case SDL_MOUSEBUTTONDOWN:
				joystick_start_x = event.button.x;
				joystick_start_y = event.button.y;
				mouse_is_down = 1;
				break;
			case SDL_MOUSEBUTTONUP:
				mouse_is_down = 0;
				break;
			case SDL_MOUSEMOTION:;
				int max_delta = 250;
				if(mouse_is_down) {
					int dx = event.motion.x - joystick_start_x;
					int dy = -(event.motion.y - joystick_start_y);
					if(dx > max_delta) dx = max_delta;
					if(dx < -max_delta) dx = -max_delta;
					if(dy > max_delta) dy = max_delta;
					if(dy < -max_delta) dy = -max_delta;

					SDL_LockMutex(packet->input_mutex);
					if(dx * dx + dy * dy > 2500) {
						packet->joystick_x = dx / (double)max_delta;
						packet->joystick_y = dy / (double)max_delta;
						packet->has_new_data = 1;
					}
					else {
						packet->joystick_x = 0;
						packet->joystick_y = 0;
						packet->has_new_data = 1;
					}
					SDL_UnlockMutex(packet->input_mutex);
				}
				break;
			}
		}
	}

	SDL_WaitThread(data->main_thread, NULL);
	SDL_WaitThread(data->compute_thread, NULL);
}

DemoWindow * create_demo_window(int width, int height) {

	DemoWindow * window = malloc(sizeof(DemoWindow));
	*window = (DemoWindow){
		width / 2,
		height / 2,
		NULL,
		NULL,
		SDL_CreateMutex(),
		calloc(width * height / 4, sizeof(int)),
		calloc(width * height / 4, sizeof(int))
	};

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
