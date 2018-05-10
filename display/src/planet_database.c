#include "planet_database.h"
#include "planet_renderer.h"
#include "vector.h"

void updateBodies(PlanetRenderer * planets) {
	fillBodyData(&planets->bodies[0],
	             5,
	             (Vector){50, 0, 0},
	             1);
	fillBodyData(&planets->bodies[0].children[0],
	             1,
	             (Vector){50, 5, 0},
	             0);
	planets->number_of_bodies = 2;
}

void fillBodyData(SolarBody * body,
                  double radius,
                  Vector pos,
                  int num_of_children) {
	*body = (SolarBody){radius,
		            pos,
		            num_of_children,
		            malloc(sizeof(SolarBody) * num_of_children)};
}
