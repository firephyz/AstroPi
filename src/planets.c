#include "planets.h"
#include "demo.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void renderScene(DisplayData * display, struct runtime_data_t * data) {

  SolarBody * target = data->bodies + data->target_body_index;
  render_body(display, data, target);
  for(int i = 0; i < target->num_of_children; ++i) {
    SolarBody * body = target->children[i];
    render_body(display, data, body);
  }

  /* for(int i = 0; i < data->num_of_bodies; ++i) { */
  /*   SolarBody * body = data->bodies + i; */
  /*   render_body(display, data, body); */
  /* } */
}

void render_body(DisplayData * display, struct runtime_data_t * data, SolarBody * body) {
  
  double aspect_ratio = (double)display->width / display->height;
    
  double scale_factor = aspect_ratio / (aspect_ratio + body->pos.z * 2 * tan(M_PI / 2 * data->cam->fov));
  int pix_x = body->pos.x / aspect_ratio * display->width * scale_factor;
  int pix_y = body->pos.y * display->height * scale_factor;
  int pix_r = body->radius * scale_factor * display->width / aspect_ratio;

  pix_x += display->width / 2;
  pix_y = display->height / 2 - pix_y;

  int color;
  /* if(body->pos.z > FADE_THRESHOLD) { */
  /*   if(body->pos.z > FADE_THRESHOLD + FADE_DEPTH) { */
  /* 	color = COLOR_BLACK; */
  /*   } */
  /*   else { */
  /* 	double ratio = 1 - (body->pos.z - FADE_THRESHOLD) / FADE_DEPTH; */
  /* 	color = NEW_COLOR((int)(0x1F * ratio), */
  /* 			  (int)(0x3F * ratio), */
  /* 			  (int)(0x1F * ratio)); */
  /*   } */
  /* } */
  /* else { */
  /*   color = COLOR_WHITE; */
  /* } */
  color = COLOR_WHITE;
  display_set_color(display, color);
  draw_planet(display,
	      pix_x,
	      pix_y,
	      pix_r);

  // Report the apparent size of the planet so
  // we can tell which way to zoom
  if(body == data->bodies + data->target_body_index) {
    printf("%2.2f\n", 2 * body->radius * scale_factor);
  }
}

void draw_planet(DisplayData * display, int x, int y, int radius) {
  
  if(x >= -radius &&
     x <= display->width + radius &&
     y >= -radius &&
     y <= display->height + radius) {
    
    // Always draw center point no matter radius
    if(in_display_bounds(display, x, y)) {
      display->buffer[x + y * display->width] = display->color;
    }
    
    int draw_radius = radius;
    while(draw_radius > 0) {
      display_draw_circle(display, x, y, draw_radius);
      draw_radius -= 5;
    }
  }
}

void fetch_planet_data(struct runtime_data_t * data) {

  for(int i = 0; i < data->num_of_bodies; ++i) {
    load_body(data, i);
    /* SolarBody * body = data->bodies + i; */
    /* printf("%s (%p)\n%f, %f, %f\n%f\n\n", */
    /* 	   body->name, */
    /* 	   body, */
    /* 	   body->pos.x, */
    /* 	   body->pos.y, */
    /* 	   body->pos.z, */
    /* 	   body->radius); */
  }
}

char * planet_names[] = {
  "sun",
  "mercury",
  "mars",
  "jupiter",
  "uranus",
  "pluto",
  "io",
  "europa",
  "ganymede",
  "callisto",
  "phobos",
  "deimos",
  "triton",
  "nereid",
  "proteus"
};

double  planet_radi[] = {
  695500,
  2440,
  3389.9,
  71492,
  25559,
  1195,
  1820,
  1560,
  2631,
  2410,
  12,
  6.3,
  1352,
  170,
  210
};
  

void load_body(struct runtime_data_t * data, int planet_index) {

  char * planet_name = planet_names[planet_index];
  char path[50];
  
  sprintf(path, "./cspice_database/planets/%s.dat", planet_name);
  FILE * file_in = fopen(path, "r");
  SolarBody * body = data->bodies + planet_index;
  
  strcpy(body->name, planet_name);
  body->radius = planet_radi[planet_index];
  body->locations = malloc(sizeof(Vector) * LOCATION_SAMPLES_PER_PLANET);
  for(int i = 0; i < LOCATION_SAMPLES_PER_PLANET; ++i) {
    Vector pos;
    fscanf(file_in, "%lf, %lf, %lf\n", &pos.x, &pos.y, &pos.z);
    body->locations[i] = pos;
  }
  body->pos = body->locations[LOCATION_SAMPLES_PER_PLANET / 2];

  switch(planet_index) {
  case 2:
    body->num_of_children = 2;
    body->children = malloc(sizeof(SolarBody *) * body->num_of_children);
    body->children[0] = data->bodies + 10;
    body->children[1] = data->bodies + 11;
    break;
  case 3:
    body->num_of_children = 4;
    body->children = malloc(sizeof(SolarBody *) * body->num_of_children);
    body->children[0] = data->bodies + 6;
    body->children[1] = data->bodies + 7;
    body->children[2] = data->bodies + 8;
    body->children[3] = data->bodies + 9;
    break;
  case 4:
    body->num_of_children = 3;
    body->children = malloc(sizeof(SolarBody *) * body->num_of_children);
    body->children[0] = data->bodies + 12;
    body->children[1] = data->bodies + 13;
    body->children[2] = data->bodies + 14;
  default:
    body->children = NULL;
    body->num_of_children = 0;
    break;
  }

  fclose(file_in);
}
