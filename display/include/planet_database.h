#ifndef PLANET_DATABASE_INCLUDED

#define PLANET_DATABASE_INCLUDED

#define MAX_NUMBER_OF_PARENT_BODIES 64

typedef struct SolarBody_t SolarBody;

#include "planet_renderer.h"
#include "vector.h"

struct SolarBody_t {
  double radius;
  Vector pos;
  int num_of_children;
  SolarBody * children;
};

void updateBodies(PlanetRenderer * planets);
void fillBodyData(SolarBody * body,
                  double radius,
                  Vector pos,
                  int num_of_children);

#endif
