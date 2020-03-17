#ifndef CONTOUR_H
#define CONTOUR_H

#include "common.h"
#include "image.h"

typedef struct {
	size_t*	Xi;
	size_t*	Yi;
	size_t 	size;
	size_t 	index;
} Contour;


Contour* 	new_contour();
void 		insert_point(Contour* cnt, size_t x, size_t y);
void 		free_contour(Contour* cnt);
Contour** 	find_contours(Image* img, size_t* index_size);
void		contour_center(Contour* cnt, float* x, float* y);

#endif
