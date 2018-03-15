#ifndef DEMO_INCLUDED

#define DEMO_INCLUDED

#include <pthread.h>
#include <stdint.h>
#include "display.h"

#define ARDUINO_CLK_PIN 29
#define ARDUINO_DATA_PIN 28
#define ARDUINO_READY_PIN 27

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320

#define LOADING_BAR_LENGTH 400
#define LOADING_BAR_WIDTH 50

#define MAX_ITER_COUNT 300

struct runtime_data_t {
  volatile int is_running;
};

void demo_mandelbrot(DisplayData * display, struct runtime_data_t * data);
pthread_t init_input_handler(struct runtime_data_t * data);
void * input_handler(void * data);
int getInputData();

void computeMandelbrot(DisplayData * display, int * escapeTimes);
void drawMandelbrot(DisplayData * display, int * escapeTimes);
uint16_t pointCountToColor(int count);
void updateLoadingBar(DisplayData * display, int index);
void computePointPosition(DisplayData * display, int x, int y, double * xPos, double * yPos);
int iteratePoint(double x, double y);
double magnitude(double x, double y);

#endif
