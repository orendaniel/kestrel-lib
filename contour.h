#ifndef CONTOUR_H
#define CONTOUR_H

#include "common.h"
#include "image.h"

struct point {
	size_t x, y;
};

typedef struct {
	struct point* 	points;
	size_t 			size, index;
} Contour;


//COMMON FUNCTIONS
//----------------------------------------------------------------------------------------------------

Contour* 	new_contour();
void 		insert_point(Contour* cnt, size_t x, size_t y);
void 		free_contour(Contour* cnt);
Contour** 	find_contours(Image* img, size_t* index_size, size_t steps_x, size_t steps_y);

//----------------------------------------------------------------------------------------------------


//CONTOUR CALCULATION
//----------------------------------------------------------------------------------------------------

void 			contour_center(Contour* cnt, float* x, float* y);
struct point* 	get_contour_extreme(Contour* cnt);
size_t 			get_contour_area(Contour* cnt);
void 			fit_line(Contour* cnt, float* m, float* b);

//----------------------------------------------------------------------------------------------------

#endif
