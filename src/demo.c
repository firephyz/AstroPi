/*****************************************
 * AstroPi ECE 387 Embedded Systems Demo *
 *                                       *
 * Kyle Burge                            *
 *****************************************/

#include "demo.h"
#include "display.h"
#include "input.h"
#include "fonts.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

Glyph * font_glyphs;

int main(int argc, char *argv[]) {

  initFontGlyphs();
  DisplayData * display = init_display();
  
  struct runtime_data_t runtime_data;
  runtime_data.bodies = malloc(sizeof(SolarBody) * INIT_NUM_BODIES);
  //  runtime_data.num_of_bodies = INIT_NUM_BODIES;
  runtime_data.num_of_bodies = INIT_NUM_BODIES;
  runtime_data.target_body_index = 3;
  runtime_data.time_index = LOCATION_SAMPLES_PER_PLANET / 2;
  runtime_data.cam = malloc(sizeof(TeleCamera));
  *runtime_data.cam = (TeleCamera){
    .pos = (Vector){0, 0, 0},
    .angle = (Vector){0, 0, 0},
    .fov = 0.00025
  };
  runtime_data.mutex = malloc(sizeof(pthread_mutex_t));
  runtime_data.is_running = 1;
  runtime_data.has_input = 0;
  runtime_data.input = (InputPacket){
    .scroll = 0,
    .buttons = 0,
    .joyx = 0,
    .joyy = 0
  };
  fetch_planet_data(&runtime_data);
  pthread_mutex_init(runtime_data.mutex, NULL);
  pthread_t * input_thread = init_input_handler(&runtime_data);

  // Run the demo
  demo_planets(display, &runtime_data);

  // Wait for input thread to respond to the program quit
  if(pthread_join(*input_thread, NULL)) {
    fprintf(stderr, "Error joining input thread.");
    exit(-1);
  }

  return 0;
}

void demo_planets(DisplayData * display, struct runtime_data_t * data) {

  struct timespec delay = {0, 200000000};

  while(1) {
    display_set_color(display, COLOR_BLACK);
    //     display_clear(display);

    process_input(display, data);
    adjust_solar_bodies(data);
    renderScene(display, data);
    drawString(display, 5, 5, "Hello Universe!");

    for(int i = 0; i < 128; ++i) {
      drawChar(display,
	       10 + FONT_WIDTH * (i % 8),
	       20 + FONT_HEIGHT * (i / 8),
	       (char)i);
    }
    
    display_update(display);
    nanosleep(&delay, NULL);
  }
}

void process_input(DisplayData * display, struct runtime_data_t * data) {

  InputPacket input;
  readRuntimeData(data, RUNTIME_INPUT, &input);

  if(input.joyx > 80) {
    data->time_index += 5;
  }
  else if(input.joyx < -80) {
    data->time_index -= 5;
  }

  double scale_factor = 0.94;
  if(input.scroll > 0) {
    data->cam->fov *= pow(scale_factor, input.scroll);
  }
  else if(input.scroll < 0) {
    data->cam->fov *= pow(2 - scale_factor, -input.scroll);    
  }
}

void adjust_solar_bodies(struct runtime_data_t * data) {

  // Update planet locations based on the current time
  for(int i = 0; i < INIT_NUM_BODIES; ++i) {
    SolarBody * body = data->bodies + i;
    body->pos = body->locations[data->time_index];
  }

  // Perform rotation transformations to put the body
  // in front of the camera
  SolarBody * target = data->bodies + data->target_body_index;
  calibrate_camera(data, target);
  // Also do the same to the children
  for(int i = 0; i < target->num_of_children; ++i) {
    transform_body(data, target->children[i]);
  }
}

void transform_body(struct runtime_data_t * data, SolarBody * body) {

  double angle = data->cam->angle.x;
  Vector pos = {body->pos.x * cos(angle) + body->pos.y * sin(angle),
		-body->pos.x * sin(angle) + body->pos.y * cos(angle),
		body->pos.z};
  body->pos = pos;

  angle = data->cam->angle.y;
  pos = (Vector){-body->pos.x * sin(angle) + body->pos.z * cos(angle),
		 body->pos.y,
		 body->pos.x * cos(angle) - body->pos.z * sin(angle)};
  body->pos = pos;  
}

void calibrate_camera(struct runtime_data_t * data, SolarBody * target) {
  data->cam->angle.x = atan(target->pos.y / target->pos.x);
  if(target->pos.x < 0) data->cam->angle.x += M_PI;
  double angle = data->cam->angle.x;
  Vector pos = {target->pos.x * cos(angle) + target->pos.y * sin(angle),
		-target->pos.x * sin(angle) + target->pos.y * cos(angle),
		target->pos.z};
  target->pos = pos;

  
  data->cam->angle.y = atan(target->pos.z / target->pos.x);
  if(target->pos.x < 0) data->cam->angle.y += M_PI;
  angle = data->cam->angle.y;
  pos = (Vector){-target->pos.x * sin(angle) + target->pos.z * cos(angle),
		 target->pos.y,
		 target->pos.x * cos(angle) - target->pos.z * sin(angle)};
  target->pos = pos;
}

DisplayData * init_display() {

  DisplayData * display = malloc(sizeof(DisplayData));
  *display = (DisplayData){fopen("/dev/fb1", "w+"),
			   COLOR_BLACK,
			   malloc(sizeof(uint16_t) * DISPLAY_WIDTH * DISPLAY_HEIGHT),
			   DISPLAY_WIDTH,
			   DISPLAY_HEIGHT,
			   DISPLAY_WIDTH * DISPLAY_HEIGHT};

  return display;
}

void readRuntimeData(struct runtime_data_t * data, int choice, void * result) {

  pthread_mutex_lock(data->mutex);
  switch(choice) {
  case RUNTIME_RUNNING:
    *(int *)result = data->is_running;
    break;
  case RUNTIME_HAS_INPUT:
    *(int *)result = data->has_input;
    break;
  case RUNTIME_INPUT:
    *(InputPacket *)result = data->input;
    break;
  default:
    fprintf(stderr, "Internal error.\n");
    exit(-1);
  }
  pthread_mutex_unlock(data->mutex);

  return;
}

void setRuntimeData(struct runtime_data_t * data, int choice, void * value) {

  pthread_mutex_lock(data->mutex);
  switch(choice) {
  case RUNTIME_RUNNING:
    data->is_running = *(int *)value;
    break;
  case RUNTIME_HAS_INPUT:
    data->has_input = *(int *)value;
    break;
  case RUNTIME_INPUT:
    data->input = *(InputPacket *)value;
    break;
  default:
    fprintf(stderr, "Internal error.\n");
    exit(-1);
  }
  pthread_mutex_unlock(data->mutex);

  return;
}
