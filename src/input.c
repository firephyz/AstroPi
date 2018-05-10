#include "input.h"
#include "demo.h"

#include <wiringPi.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

void * input_handler(void * vdata) {

  struct runtime_data_t * data = (struct runtime_data_t *)vdata;

  while(1) {
    int is_running = 1;
    readRuntimeData(data, RUNTIME_RUNNING, &is_running);

    if(!is_running) break;
    
    /* if(digitalRead(ARDUINO_READY_PIN) == 1) { */
    /*   InputPacket input = getInputData(); */
    /*   setRuntimeData(data, RUNTIME_INPUT, &input); */
    /*   printInputData(input); */
    /* } */

    struct timespec delay = {0, 10000000};
    nanosleep(&delay, NULL);
  }

  return NULL;
}

InputPacket getInputData() {

  unsigned int result = 0;
  struct timespec data_delay = {0, 1250000};
  
  for(int i = 0; i < 4; ++i) {
    digitalWrite(ARDUINO_CLK_PIN, 1);
    nanosleep(&data_delay, NULL);
    unsigned int data_in = digitalRead(ARDUINO_DATA_PIN) == 1 ? 0x80000000 : 0;
    result = result | data_in;
    result = result >> 1;
    digitalWrite(ARDUINO_CLK_PIN, 0);
    nanosleep(&data_delay, NULL);    
  }

  result = result >> 27;
  
  return (InputPacket){result, 0, 0, 0, 0};
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

  digitalWrite(ARDUINO_CLK_PIN, 0);

  pthread_t * input_thread = malloc(sizeof(pthread_t));
  if(pthread_create(input_thread, NULL, input_handler, data)) {
    fprintf(stderr, "Error creating input thread.");
    exit(-1);
  }

  return input_thread;
}

void printInputData(InputPacket data) {
  printf("%10s %d\n%10s (%d, %d)\n%10s (%1.2f, %1.2f)\n",
	 "Scroll: ",
	 data.scroll,
	 "Buttons:",
	 data.button1,
	 data.button2,
	 "Joystick:",
	 data.joyx,
	 data.joyy);
}
