#ifndef DEMO_INCLUDED

#define DEMO_INCLUDED

#include "display.h"
#include "input.h"

#include <stdint.h>
#include <pthread.h>

// Constants for reading values from the runtime_data_t struct.
// Used by readRuntimeData and setRuntimeData functions.
#define RUNTIME_RUNNING 0
#define RUNTIME_HAS_INPUT 1
#define RUNTIME_INPUT 2

typedef struct input_packet_t InputPacket;
typedef struct solar_body_t SolarBody;
typedef struct telecamera_t TeleCamera;

typedef struct demo_info_t DemoInfo;

struct demo_info_t {
  double apparent_size;
  char * body_name;
  double * fov;
};

struct runtime_data_t {
  SolarBody * bodies;
  int num_of_bodies;
  int target_body_index;
  int time_index;

  DemoInfo info;
  TeleCamera * cam;

  // Shared data with input thread
  pthread_mutex_t * mutex;
  volatile int is_running;
  volatile int has_input;
  volatile InputPacket input;
};

#include "planets.h"

void demo_planets(DisplayData * display, struct runtime_data_t * data);
void readRuntimeData(struct runtime_data_t * data, int choice, void * result);
void setRuntimeData(struct runtime_data_t * data, int choice, void * value);
void init_runtime_data(struct runtime_data_t * runtime_data);


/* // Transform the target planet to be focused in the exact center of the camera. */
/* // This also sets the camera angles. */
/* #define TRANSFORM_FOCUS 0 */
/* // Transform the target planet to be focused in relation to the camera angle */
/* // (thereby bringing it in frame) */
/* #define TRANSFORM_IN_FRAME 1 */

void adjust_solar_bodies(struct runtime_data_t * data);
void transform_body(struct runtime_data_t * data, SolarBody * body);
void calibrate_camera(struct runtime_data_t * data, SolarBody * target);
void process_input(DisplayData * display, struct runtime_data_t * data);
void display_information(DisplayData * display, struct runtime_data_t * data);
void format_date_string(int time_index, char * string);

#endif
