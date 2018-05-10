#include "planet_renderer.h"
#include "demo.h"
#include "fonts.h"

void processPlanetEvents(PlanetRenderer * planets,
                         EventPacket * input_events) {
	EventPacket packet;
	SDL_LockMutex(input_events->input_mutex);
	if(input_events->has_new_data) {
		packet = *input_events;
		input_events->has_new_data = 0;
	}
	else {
		packet.has_new_data = 0;
	}
	SDL_UnlockMutex(input_events->input_mutex);

	if(packet.has_new_data) {
		// Process events here
		// printf("Scroll: %d\n", packet.scroll);
		// printf("Button1: %d\n", packet.button1);
		// printf("Button2: %d\n", packet.button2);
		// printf("Joystick: %1.2f, %1.2f\n", packet.joystick_x, packet.joystick_y);
	}
	return;
}
void renderPlanets(PlanetRenderer * planets, DemoWindow * window) {

	// Divide by 2 since each draw pixel is
	// actually composed of a 2x2 rectangle
	//int (*buffer)[window->width] = (int (*)[window->width])window->draw_buffer;



	return;
}

void renderMenu(DemoWindow * window) {

	int (*buffer)[window->width] = (int (*)[window->width])window->draw_buffer;

	for(int i = 0; i < window->width; ++i) {
		buffer[0][i] = 1;
		buffer[window->height - 1][i] = 1;
		buffer[window->height - 2][i] = 1;
	}
	for(int i = 0; i < window->height; ++i) {
		buffer[i][0] = 1;
		buffer[i][1] = 1;
		buffer[i][window->width - 1] = 1;
		buffer[i][window->width - 2] = 1;
	}

	char * title_string = " AstroPi Solar System Explorer ";
	int title_length = strlen(title_string);
	int title_box_height = FONT_HEIGHT + 3;
	int title_box_width = FONT_WIDTH * title_length + 2;
	int title_box_x_start = window->width / 2 - title_box_width / 2;
	int title_box_x_end = window->width / 2 + title_box_width / 2;
	for(int y = 0; y <= title_box_height; ++y) {
		for(int x = 0; x < window->width - 1; ++x) {
			if(y == 0 || y >= title_box_height - 1 || (x < title_box_x_start || x >= title_box_x_end)) {
				buffer[y + 1][x + 1] = 1;
			}
			else {
				buffer[y + 1][x + 1] = 0;
			}
		}
	}

	drawString(window, title_box_x_start + 1, 3, title_string);
}
