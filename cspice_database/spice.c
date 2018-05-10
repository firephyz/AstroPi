/*
 * Helper file for the AstroPi. SPICE can't
 * run natively on the Pi so we must create some
 * databases externally on a PC. 
 *
 * * Note. Expects necessary SPICE kernels in  ./kernels/
 *
 * Kyle Burge
 */

#include <stdio.h>
#include <stdlib.h>
#include "SpiceUsr.h"

#define NUM_OF_BODIES 15
char *names[NUM_OF_BODIES] = {
  "sun",
  "mercury",
  "mars barycenter",
  "jupiter barycenter",
  "saturn barycenter",
  "pluto barycenter",
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

int main(int argc, char *argv[]) {

  furnsh_c("./kernels/de430.bsp");
  furnsh_c("./kernels/mar097.bsp");
  furnsh_c("./kernels/jup310.bsp");
  furnsh_c("./kernels/nep081.bsp");
  furnsh_c("./kernels/naif0012.tls");

  SpiceDouble time_window = 24 * 30 * 24 * 3600;
  SpiceDouble current_time;
  str2et_c("2018/5/10 18:00:00.00", &current_time);

  int i;
  for(i = 0; i < NUM_OF_BODIES; ++i) {
    char first_name[30];
    char second_name[30];
    sscanf(names[i], "%s %s", first_name, second_name);
    char file_name[50];
    sprintf(file_name, "planets/%s.dat", first_name);
    FILE * out_file = fopen(file_name, "w");
    
    SpiceDouble et = current_time - time_window / 2;
    while(et < current_time + time_window / 2) {
      SpiceDouble data[6];
      SpiceDouble lt;
      
      spkezr_c(names[i], et,  "eclipJ2000",
	       "NONE", "earth", data, &lt);
      fprintf(out_file,
	      "%f, %f, %f\n",
	      data[0],
	      data[1],
	      data[2]);

      et += 3600;
    }
    
    fclose(out_file);
  }

  return 0;
}
