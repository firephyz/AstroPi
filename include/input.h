#ifndef INPUT_INCLUDED

#define INPUT_INCLUDED

#include <pthread.h>

#define ARDUINO_CLK_PIN 29
#define ARDUINO_DATA_PIN 28
#define ARDUINO_READY_PIN 27
#define ARDUINO_ACK_PIN 26

#include <stdint.h>

struct input_packet_t{
  int8_t scroll;
  int8_t joyx;
  int8_t joyy;
  uint8_t buttons;
} __attribute__((packed));
typedef struct input_packet_t InputPacket;

void * input_handler(void * vdata);

// Argument really is struct runtime_data_t *
// Declared as void to avoid strange forward declaration trickery
pthread_t * init_input_handler(void * data);

void getInputData(InputPacket * input);
void printInputData(InputPacket * data);

#endif
