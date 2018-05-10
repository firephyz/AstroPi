#ifndef INPUT_INCLUDED

#define INPUT_INCLUDED

#include <pthread.h>

#define ARDUINO_CLK_PIN 29
#define ARDUINO_DATA_PIN 28
#define ARDUINO_READY_PIN 27

struct input_packet_t{
  int scroll;
  int button1;
  int button2;
  double joyx;
  double joyy;
};
typedef struct input_packet_t InputPacket;

void * input_handler(void * vdata);

// Argument really is struct runtime_data_t *
// Declared as void to avoid strange forward declaration trickery
pthread_t * init_input_handler(void * data);

InputPacket getInputData();
void printInputData(InputPacket data);

#endif
