#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <wiringPi.h>
#include "demo.h"
#include "display.h"

volatile int iter_cap = MAX_ITER_COUNT / 2;
volatile int should_render = 0;

int main(int argc, char *argv[]) {

  DisplayData display = {fopen("/dev/fb1", "w+"),
			 COLOR_BLACK,
			 malloc(sizeof(uint16_t) * DISPLAY_WIDTH * DISPLAY_HEIGHT),
			 DISPLAY_WIDTH,
			 DISPLAY_HEIGHT,
			 DISPLAY_WIDTH * DISPLAY_HEIGHT};

  struct runtime_data_t runtime_data = {1};
  pthread_t input_thread = init_input_handler(&runtime_data);

  // Run the demo
  demo_mandelbrot(&display, &runtime_data);

  // Wait for input thread to respond to the program quit
  if(pthread_join(input_thread, NULL)) {
    fprintf(stderr, "Error joining input thread.");
    exit(-1);
  }

  return 0;
}

void * input_handler(void * vdata) {

  struct runtime_data_t * data = (struct runtime_data_t *)vdata;

  while(data->is_running) {
    if(digitalRead(ARDUINO_READY_PIN) == 1) {
      int input = getInputData();
      if(input < 8) {
	iter_cap -= 5;
      }
      else {
	iter_cap += 5;
      }
      should_render = 1;
    }
    usleep(50000);
  }

  return NULL;
}

int getInputData() {

  unsigned int result = 0;
  for(int i = 0; i < 4; ++i) {
    digitalWrite(ARDUINO_CLK_PIN, 1);
    usleep(1250);
    unsigned int data_in = digitalRead(ARDUINO_DATA_PIN) == 1 ? 0x80000000 : 0;
    result = result | data_in;
    result = result >> 1;
    digitalWrite(ARDUINO_CLK_PIN, 0);
    usleep(1250);
  }

  result = result >> 27;
  return result;
}

void demo_mandelbrot(DisplayData * display, struct runtime_data_t * data) {

  display_set_color(display, COLOR_BLACK);
  display_clear(display);
  display_set_color(display, COLOR_WHITE);
  display_draw_rect(display,
		    (display->width - LOADING_BAR_LENGTH - 2) / 2,
		    (display->height - LOADING_BAR_WIDTH - 2) / 2,
		    LOADING_BAR_LENGTH + 2,
		    LOADING_BAR_WIDTH + 2);
  display_update(display);

  // Precompute all mandelbrot escape times.
  int escapeTimes[display->pixCount];
  computeMandelbrot(display, escapeTimes);

  drawMandelbrot(display, escapeTimes);

  // Display the results and then wait for input to change the display
  while(data->is_running) {
    if(should_render) {
      drawMandelbrot(display, escapeTimes);
      should_render = 0;
    }
    usleep(100000);
  }
}

void drawMandelbrot(DisplayData * display, int * escapeTimes) {

  for(int y = 0; y < display->height; ++y) {
    for(int x = 0; x < display->width; ++x) {
      int index = x + y * display->width;
      int count = escapeTimes[index];
      display->buffer[index] = pointCountToColor(count);
    }
  }

  display_update(display);
}

uint16_t pointCountToColor(int count) {

  if(count > iter_cap) count = iter_cap;
  double ratio = (double)count / iter_cap;
  int r = (int)(0x1F * ratio);
  int g = (int)(0x3F * ratio);
  int b = (int)(0x1F * ratio);
  return NEW_COLOR(r, g, b);
}

void computeMandelbrot(DisplayData * display, int * escapeTimes) {

  // Set progress bar color
  display_set_color(display, COLOR_GREY);

  clock_t startTime = clock();
  for(int y = 0; y < display->height; ++y) {
    for(int x = 0; x < display->width; ++x) {
      // Compute escape count for each point
      double xPos, yPos;
      if(y == 160) {
	xPos = 1;
      }
      computePointPosition(display, x, y, &xPos, &yPos);
      int count = iteratePoint(xPos, yPos);

      // Store that count
      escapeTimes[x + y * display->width] = count;

      int deltaTime = clock() - startTime;
      if(deltaTime > CLOCKS_PER_SEC / 5) {
	startTime += deltaTime;
	updateLoadingBar(display, y * display->width + x);
      }
    }
  }
  
  updateLoadingBar(display, display->pixCount);
}

int iteratePoint(double x, double y) {

  double cx = x;
  double cy = y;
  int count = 0;

  while(magnitude(x, y) < 2) {
    if(count >= MAX_ITER_COUNT) {
      break;
    }
    
    ++count;

    double new_x = x * x - y * y + cx;
    double new_y = 2 * x * y + cy;
    x = new_x;
    y = new_y;
  }

  return count;
}

double magnitude(double x, double y) {

  return sqrt(x * x + y * y);
}

void computePointPosition(DisplayData * display,
			  int x,
			  int y,
			  double * xPos,
			  double * yPos) {
  *xPos = -2.75 + (double)x / display->width * 4;
  *yPos = (4.0 / 3) - (double)y / display->height * 8 / 3;
}

void updateLoadingBar(DisplayData * display, int index) {

  double percentage = (double)index / display->pixCount;
  int progressLength = (int)(LOADING_BAR_LENGTH * percentage);
  display_fill_rect(display,
		    (display->width - LOADING_BAR_LENGTH) / 2,
		    (display->height - LOADING_BAR_WIDTH) / 2,
		    progressLength,
		    LOADING_BAR_WIDTH);
  display_update(display);
}

pthread_t init_input_handler(struct runtime_data_t * data) {

  // Setup wiringPi and the pins we need to use.
  if(wiringPiSetup() == -1) {
    fprintf(stderr, "Error setting up wiringPi library.");
    exit(-1);
  }

  pinMode(ARDUINO_READY_PIN, INPUT);
  pinMode(ARDUINO_CLK_PIN, OUTPUT);
  pinMode(ARDUINO_DATA_PIN, INPUT);

  digitalWrite(ARDUINO_CLK_PIN, 0);

  pthread_t input_thread;
  if(pthread_create(&input_thread, NULL, input_handler, data)) {
    fprintf(stderr, "Error creating input thread.");
    exit(-1);
  }

  return input_thread;
}
