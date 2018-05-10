#ifndef VECTOR_INCLUDED

#define VECTOR_INCLUDED

typedef struct {
	double x;
	double y;
	double z;
} Vector;

void vector_add(Vector * dest, Vector * a, Vector * b);
void vector_sub(Vector * dest, Vector * a, Vector * b);
double vector_dot(double * dest, Vector * a, Vector * b);
void vector_cross(Vector * dest, Vector * a, Vector * b);
double vector_mag(double * dest, Vector * a);
double vector_angle(double * dest, Vector * a, Vector * b);

#endif