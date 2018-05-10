#include "vector.h"
#include <math.h>

void vector_add(Vector * dest, Vector * a, Vector * b) {
	double * a_data = (double *)a;
	double * b_data = (double *)b;
	double * dest_data = (double *)dest;
	for(int i = 0; i < 3; ++i) {
		dest_data[i] = a_data[i] + b_data[i];
	}
}

void vector_sub(Vector * dest, Vector * a, Vector * b) {
	double * a_data = (double *)a;
	double * b_data = (double *)b;
	double * dest_data = (double *)dest;
	for(int i = 0; i < 3; ++i) {
		dest_data[i] = a_data[i] - b_data[i];
	}
}

double vector_dot(double * dest, Vector * a, Vector * b) {
	double result = 0;
	double * a_data = (double *)a;
	double * b_data = (double *)b;
	for(int i = 0; i < 3; ++i) {
		result += a_data[i] * b_data[i];
	}
	if(dest) *dest = result;
	return result;
}

void vector_cross(Vector * dest, Vector * a, Vector * b) {
	dest->x = a->y * b->z - a->z * b->y;
	dest->y = a->z * b->x - a->x * b->z;
	dest->z = a->x * b->y - a->y * b->x;
}

double vector_mag(double * dest, Vector * a) {
	double result = 0;
	vector_dot(&result, a, a);
	result = sqrt(result);
	if(dest) *dest = result;
	return result;
}

double vector_angle(double * dest, Vector * a, Vector * b) {
	double result = 0;
	double a_mag, b_mag;
	vector_dot(&result, a, b);
	vector_mag(&a_mag, a);
	vector_mag(&b_mag, b);
	result = acos(result / a_mag / b_mag);
	if(dest) *dest = result;
	return result;
}

/*
Anything like this on the market?
Why use the components we used?
Explain within 3 minutes.
*/