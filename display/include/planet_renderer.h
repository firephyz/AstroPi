#ifndef PLANET_RENDERER_INCLUDED

#define PLANET_RENDERER_INCLUDED

typedef struct PlanetRenderer_t PlanetRenderer;

#include "demo.h"
#include "vector.h"
#include "planet_database.h"

typedef struct {
  double depth;
  Vector rotation;
} TeleCamera;

struct PlanetRenderer_t {
  int number_of_bodies;
  SolarBody * bodies;
  TeleCamera camera;
};

void processPlanetEvents(PlanetRenderer * planets,
                         EventPacket * input_events);
void renderPlanets(PlanetRenderer * planets, DemoWindow * window);
void renderMenu(DemoWindow * window);

#endif
