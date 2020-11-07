/*
Kestrel vision library
Copyright (C) 2020  Oren Daniel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONTOUR_H
#define CONTOUR_H

#include "common.h"
#include "image.h"

struct point {
	size_t x, y;
};

struct stack{
	size_t 			max;
	size_t 			size;
	struct point* 	items;
};

typedef struct {
	struct point* 	points;
	size_t 			size, index;
} Contour;


// COMMON FUNCTIONS
//----------------------------------------------------------------------------------------------------

Contour* 	make_contour();
void 		insert_point(Contour* cnt, size_t x, size_t y);
void 		free_contour(Contour* cnt);
Contour** 	find_contours(Image* img, size_t* index_size, size_t steps_x, size_t steps_y);

//----------------------------------------------------------------------------------------------------


// CONTOUR CALCULATION
//----------------------------------------------------------------------------------------------------

void 			contour_center(Contour* cnt, float* x, float* y);
struct point* 	get_contour_extreme(Contour* cnt);
size_t 			get_contour_area(Contour* cnt);
void 			fit_line(Contour* cnt, float* m, float* b);

//----------------------------------------------------------------------------------------------------

#endif
