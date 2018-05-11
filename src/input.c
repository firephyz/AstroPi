#include "input.h"
#include "demo.h"

#include <wiringPi.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

void * input_handler(void * vdata) {

  struct runtime_data_t * data = (struct runtime_data_t *)vdata;
  InputPacket input;

  while(1) {
    int is_running = 1;
    readRuntimeData(data, RUNTIME_RUNNING, &is_running);

    if(!is_running) break;
    
    if(digitalRead(ARDUINO_READY_PIN) == 1) {
      digitalWrite(ARDUINO_ACK_PIN, 1);
      getInputData(&input);
      digitalWrite(ARDUINO_ACK_PIN, 0);
      
      setRuntimeData(data, RUNTIME_INPUT, &input);
    }
    else {
      digitalWrite(ARDUINO_ACK_PIN, 0);
    }

    struct timespec delay = {0, 500000};
    nanosleep(&delay, NULL);
  }

  return NULL;
}

void getInputData(InputPacket * input) {

  uint8_t result[4] = {0, 0, 0, 0};
  struct timespec clock_delay = {0, 10000};

  for(int i = 0; i < 8 * sizeof(result); ++i) {
    int index = i / 8;

    // Let the Arduino know to put data on the line
    digitalWrite(ARDUINO_CLK_PIN, 1);
    nanosleep(&clock_delay, NULL);

    // Falling edge of clk. Grad the data.
    digitalWrite(ARDUINO_CLK_PIN, 0);
    nanosleep(&clock_delay, NULL);

    int bit_in = digitalRead(ARDUINO_DATA_PIN) ? 0x80 : 0;
    result[index] = (result[index] >> 1) | bit_in;
  }

  *input = *(InputPacket *)result;
}

pthread_t * init_input_handler(void * vdata) {

  struct runtime_data_t * data = (struct runtime_data_t *)vdata;

  // Setup wiringPi and the pins we need to use.
  if(wiringPiSetup() == -1) {
    fprintf(stderr, "Error setting up wiringPi library.");
    exit(-1);
  }

  pinMode(ARDUINO_READY_PIN, INPUT);
  pinMode(ARDUINO_CLK_PIN, OUTPUT);
  pinMode(ARDUINO_DATA_PIN, INPUT);
  pinMode(ARDUINO_ACK_PIN, OUTPUT);

  digitalWrite(ARDUINO_CLK_PIN, 0);
  digitalWrite(ARDUINO_ACK_PIN, 0);

  pthread_t * input_thread = malloc(sizeof(pthread_t));
  if(pthread_create(input_thread, NULL, input_handler, data)) {
    fprintf(stderr, "Error creating input thread.");
    exit(-1);
  }

  return input_thread;
}

void printInputData(InputPacket * data) {
  printf("\n\n%10s %d\n%10s %d\n%10s (%3d, %3d)\n",
	 "Scroll: ",
	 data->scroll,
	 "Buttons:",
	 data->buttons,
	 "Joystick:",
	 data->joyx,
	 data->joyy);
  printf("\033[0;0H");
}
