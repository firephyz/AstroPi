/*****************************************
 * AstroPi ECE 387 Embedded Systems Demo *
 *                                       *
 * Kyle Burge                            *
 *****************************************/

#define SHOULD_CLEAR_TRAILS 0

#include "demo.h"
#include "display.h"
#include "input.h"
#include "fonts.h"
#include "planets.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

Glyph * font_glyphs;

int main(int argc, char *argv[]) {

  initFontGlyphs();
  DisplayData * display = init_display();

  // Show starting screen notification.
  display_set_color(display, COLOR_BLACK);
  display_clear(display);
  char * loading_string_1 = "Loading NAIF SPICE Database";
  char * loading_string_2 = "Please wait...";
  drawString(display,
	     (display->width - strlen(loading_string_1) * FONT_WIDTH) / 2,
	     (display->height - FONT_HEIGHT) / 2 - FONT_HEIGHT,
	     loading_string_1);
  drawString(display,
	     (display->width - strlen(loading_string_2) * FONT_WIDTH) / 2,
	     (display->height - FONT_HEIGHT) / 2,
	     loading_string_2);
  display_update(display);

  // Initialize runtime data
  struct runtime_data_t runtime_data;
  init_runtime_data(&runtime_data);
  
  // Load the SPICE database
  fetch_planet_data(display, &runtime_data);

  // Start the input thread
  pthread_mutex_init(runtime_data.mutex, NULL);
  pthread_t * input_thread = init_input_handler(&runtime_data);

  // Run the demo
  set_target_body(&runtime_data, 2);
  display_set_color(display, COLOR_BLACK);
  display_clear(display);
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
    if(SHOULD_CLEAR_TRAILS) display_clear(display);

    process_input(display, data);
    adjust_solar_bodies(data);
    renderScene(display, data);
    display_information(display, data);
    
    display_update(display);
    nanosleep(&delay, NULL);
  }
}

void display_information(DisplayData * display, struct runtime_data_t * data) {

  static char * title = "AstroPi Solar System Explorer";
  drawString(display,
	     (display->width - strlen(title) * FONT_WIDTH) / 2,
	     1 * FONT_HEIGHT + 3,
	     title);
  for(int i = 0; i < display->width; ++i) {
    display->buffer[i + (2 * FONT_HEIGHT + 5) * display->width] = COLOR_WHITE;
  }

  char fov_string[30];
  double degrees = *data->info.fov * 180;
  int d0 = (int)degrees;
  int d1 = (int)(60 * (degrees - d0));
  double d2 = 60 * (degrees - d0 - d1 / 60);
  sprintf(fov_string,
	  "FOV: %3d\x05 %2d' %2.2f\"",
	  d0,
	  d1,
	  d2);
  drawString(display,
	     5,
	     5,
	     fov_string);

  char date_string[40];
  format_date_string(data->time_index, date_string);
  drawString(display,
	     display->width - strlen(date_string) * FONT_WIDTH,
	     5,
	     date_string);

  char body_name[30];
  strcpy(body_name, data->info.body_name);
  body_name[0] += 'A' - 'a';
  drawString(display,
	     10,
	     3 * FONT_HEIGHT,
	     body_name);
  SolarBody * body = data->bodies + data->target_body_index;
  for(int i = 0; i < body->num_of_children; ++i) {
    sprintf(body_name, "  - %s", body->children[i]->name);
    drawString(display,
	       10,
	       2 + (4 + i) * FONT_HEIGHT,
	       body_name);
  }
  char distance_string[30];
  double distance = sqrt(body->pos.x * body->pos.x +
			 body->pos.y * body->pos.y +
			 body->pos.z * body->pos.z);
  int exponent = (int)(log(distance) / log(10));
  double coef = distance / pow(10, exponent);
  sprintf(distance_string, "Distance: %1.4f e%d", coef, exponent);
  drawString(display,
	     10,
	     9 * FONT_HEIGHT,
	     distance_string);
}

void format_date_string(int time_index, char * string) {

  long d_seconds = 10 * 60 * (time_index - (LOCATION_SAMPLES_PER_PLANET / 2));
  long start_time = 573746400 + d_seconds;

  int seconds = start_time % 60;
  start_time = (start_time - seconds) / 60;
  int minutes = start_time % 60;
  start_time = (start_time - minutes) / 60;
  int hours = start_time % 24;
  start_time = (start_time - hours) / 24;
  int days = start_time % 30;
  start_time = (start_time - days) / 30;
  int months = start_time % 12;
  start_time = (start_time - months) / 12;
  int years = start_time + 2000;

  sprintf(string,
	  "%4d-%02d-%02d %02d:%02d:%02d ",
	  years,
	  months,
	  days,
	  hours,
	  minutes,
	  seconds);
}

void process_input(DisplayData * display, struct runtime_data_t * data) {

  InputPacket input;
  int switched_bodies = 0;
  int zoomed = 0;
  readRuntimeData(data, RUNTIME_INPUT, &input);
  //  printf("%d\n", data->input.scroll);

  int deadzone = 10;
  static int zooming; // Used to see if we should accelerate time warp speed
  if(input.joyx > deadzone) {
    data->time_index += zooming;
    if(input.joyx > 90) {
      ++zooming;
    }
  }
  else if(input.joyx < -deadzone) {
    data->time_index -= zooming;
    if(input.joyx < -90) {
      ++zooming;
    }
  }
  else {
    zooming = 1;
  }

  double scale_factor = 0.94;
  if(input.scroll > 0) {
    data->cam->fov *= pow(scale_factor, input.scroll);
    zoomed = 1;
  }
  else if(input.scroll < 0) {
    data->cam->fov *= pow(2 - scale_factor, -input.scroll);
    zoomed = 1;
  }
  input.scroll = 0;

  static int old_buttons = 0;
  int button_1_edge = !(old_buttons & 0x01) && (input.buttons & 0x01);
  int button_2_edge = !(old_buttons & 0x02) && (input.buttons & 0x02);
  if(button_1_edge) {
    data->target_body_index += 1;
    switched_bodies = 1;
  }
  else if(button_2_edge) {
    data->target_body_index -= 1;
    switched_bodies = 1;
  }

  old_buttons = input.buttons;
  set_target_body(data, data->target_body_index);
  // Logic seems reversed but if we don't want to clear trails,
  // then we have to clear the display to switch bodies
  if((switched_bodies || zoomed) && !SHOULD_CLEAR_TRAILS) {
    display_set_color(display, COLOR_BLACK);
    display_clear(display);
  }

  // Reset the scroll value since the input handler doesn't.
  pthread_mutex_lock(data->mutex);
  data->input.scroll = 0;
  pthread_mutex_unlock(data->mutex);
}

void adjust_solar_bodies(struct runtime_data_t * data) {

  // Update planet locations based on the current time
  for(int i = 0; i < INIT_NUM_BODIES; ++i) {
    SolarBody * body = data->bodies + i;
    body->pos = body->locations[data->time_index];
  }

  // Perform rotation transformations to put the body
  // in front of the camera
  /* SolarBody * target = data->bodies + data->target_body_index; */
  /* calibrate_camera(data, target); */
  /* // Also do the same to the children */
  /* for(int i = 0; i < target->num_of_children; ++i) { */
  /*   transform_body(data, target->children[i]); */
  /* } */
  SolarBody * target = data->bodies + data->target_body_index;
  for(int i = 0; i < INIT_NUM_BODIES; ++i) {
    SolarBody * body = data->bodies + i;
    if(target == body) {
      calibrate_camera(data, target);
    }
    else {
      transform_body(data, body);
    }
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
  double temp = pos.x;
  pos.x = pos.y;
  pos.y = temp;
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

void init_runtime_data(struct runtime_data_t * runtime_data) {
  runtime_data->bodies = malloc(sizeof(SolarBody) * INIT_NUM_BODIES);
  //  runtime_data.num_of_bodies = INIT_NUM_BODIES;
  runtime_data->num_of_bodies = INIT_NUM_BODIES;
  runtime_data->target_body_index = 0;
  runtime_data->time_index = LOCATION_SAMPLES_PER_PLANET / 2;
  runtime_data->cam = malloc(sizeof(TeleCamera));
  runtime_data->info = (DemoInfo){
    .apparent_size = 0,
    .body_name = NULL,
    .fov = &runtime_data->cam->fov
  };
  *runtime_data->cam = (TeleCamera){
    .pos = (Vector){0, 0, 0},
    .angle = (Vector){0, 0, 0},
    .fov = 0.00025
  };
  runtime_data->mutex = malloc(sizeof(pthread_mutex_t));
  runtime_data->is_running = 1;
  runtime_data->has_input = 0;
  runtime_data->input = (InputPacket){
    .scroll = 0,
    .buttons = 0,
    .joyx = 0,
    .joyy = 0
  };
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
  case RUNTIME_INPUT:;
    InputPacket * packet = (InputPacket *)value;
    data->input.scroll += packet->scroll;
    data->input.joyx = packet->joyx;
    data->input.joyy = packet->joyy;
    data->input.buttons = packet->buttons;
    break;
  default:
    fprintf(stderr, "Internal error.\n");
    exit(-1);
  }
  pthread_mutex_unlock(data->mutex);

  return;
}
