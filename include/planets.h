#ifndef PLANETS_INCLUDED

#define PLANETS_INCLUDED

#define INIT_NUM_BODIES 15
#define LOCATION_SAMPLES_PER_PLANET 17280

#define FADE_THRESHOLD 100.0
#define FADE_DEPTH 50.0

typedef struct solar_body_t SolarBody;
typedef struct telecamera_t TeleCamera;

#include "vector.h"
#include "demo.h"

struct solar_body_t {
  char name[25];
  Vector pos;
  Vector * locations;
  double radius;

  SolarBody ** children;
  int num_of_children;
};

struct telecamera_t {
  Vector pos;
  Vector angle; //x is xy rotation, y is yz, rotation. z not used.
  double fov;
};

void renderScene(DisplayData * display, struct runtime_data_t * data);
void draw_planet(DisplayData * display, int x, int y, int radius);

void fetch_planet_data(struct runtime_data_t * data);
void load_body(struct runtime_data_t * data, int planet_index);

#endif
