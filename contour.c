#include "contour.h"

#define IS_BORDER(IMAGE, X, Y) !(get_at(IMAGE, 0, X+1, Y, 0) && \
	get_at(IMAGE, 0, X, Y+1, 0) && \
	get_at(IMAGE, 0, X-1, Y, 0) && \
	get_at(IMAGE, 0, X, Y-1, 0))

//HELPERS
//----------------------------------------------------------------------------------------------------

Contour* square_trace(size_t st_x, size_t st_y, Image* img, Image* buffer) {
	Contour* cnt = new_contour();

	insert_point(cnt, st_x, st_y);

	set_at(buffer, 0, st_x, st_y, 1);

	size_t next_step_x = 0;//go left
	size_t next_step_y = -1;

	size_t nx_x = st_x + next_step_x;
	size_t nx_y = st_y + next_step_y;
	
	/*
	counter should not exceed 8
	because a square has 8 neighbours we don't want to incircle the square in an infinite loop
	*/
	int safty_counter = 0;


	while (!(nx_x == st_x && nx_y == st_y) && safty_counter < 8) {
		if (get_at(img, 0, nx_x, nx_y, 0) == 0) {

			//these lines enable 4 connectivity
			nx_x -= next_step_x;
			nx_y -= next_step_y;

			size_t tmp 	= next_step_x;
			next_step_x = -next_step_y;//go right
			next_step_y = tmp;
			
			nx_x += next_step_x;
			nx_y += next_step_y;
			safty_counter++;
		}
		else {
			insert_point(cnt, nx_x, nx_y);
			set_at (buffer, 0, nx_x, nx_y, 1);

			size_t tmp 	= next_step_x;
			next_step_x = next_step_y;//go left
			next_step_y = -tmp;

			nx_x += next_step_x;
			nx_y += next_step_y;
			
			safty_counter = 0;	
		}
			
	}
	//clean noises and unwanted sparkles
	if (cnt->index >= 8)
		return cnt;
	else { 
		for (int i = 0; i < cnt->index; i++) 
			set_at(buffer, 0, cnt->points[i].x, cnt->points[i].y, 0);// delete contour
		return NULL;
	}

}

//----------------------------------------------------------------------------------------------------

//COMMON FUNCTIONS
//----------------------------------------------------------------------------------------------------

Contour* new_contour() {
	size_t size = CONTOUR_ALLOCATION_SIZE;

	struct point* points = calloc(size, sizeof(struct point));

	if (points != 0) {
		Contour* cnt = malloc(sizeof(Contour));

		cnt->points = points;
		cnt->size 	= size;
		cnt->index 	= 0;
		return cnt;
	}
	else { //allocation failed!
		fprintf(stderr, "Cannot allocate contour\n");
		return NULL;
	}
}

void insert_point(Contour* cnt, size_t x, size_t y) {
	if (cnt->index < cnt->size) {
		struct point p = {x, y};
		cnt->points[cnt->index] = p;

		cnt->index++;
	}
	else {
		struct point* tmp = realloc(cnt->points, sizeof(struct point) * (cnt->size + CONTOUR_ALLOCATION_SIZE));
		if (tmp != 0) {
			cnt->points = tmp;
			cnt->size += CONTOUR_ALLOCATION_SIZE;
			struct point p = {x, y};
			cnt->points[cnt->index] = p;

			cnt->index++;
		}
		else {
			fprintf(stderr, "Cannot resize contour\n");
		}
	}
}

void free_contour(Contour* cnt) {
	free(cnt->points);
	free(cnt);
}


/*
find the contours in a binary image using square trace with 8 connectivity
set steps to 1 for full precision at the price of speed.
recommended 3 steps for regular images
*/
Contour** find_contours(Image* img, size_t* index_size, size_t steps_x, size_t steps_y) {
	if (img->channels == 1) {

		Image* 		buffer 	= new_image(1, img->width, img->height); //keep track of points on other contours
		Contour** 	cnts 	= malloc(sizeof(Contour**));
		size_t 		amount 	= 0;

		for (size_t i = 0; i < img->height; i += steps_y) {
			for (size_t j = 0; j < img->width; j += steps_x) {
				if (get_at(img, 0, j, i, 0) != 0 &&
						get_at (buffer, 0, j, i, 0) == 0 &&
						IS_BORDER(img, j, i)) { //only borders remain

					Contour* cnt = square_trace(j, i, img, buffer); //traces the border
					if (cnt) {
						Contour** tmp_cnts = realloc(cnts, (amount +1) * sizeof(Contour*));
						if (tmp_cnts != 0) {
							cnts = tmp_cnts;//check if need free cnts before
							cnts[amount] = cnt;
							amount++;
						}
					}
				}
			}
		}

		*index_size = amount;

		free_image(buffer);
		return cnts;
	}
	else {
		fprintf(stderr, "Contours for one channel images only\n");
		*index_size = 0;
		return NULL;
	}
}

//----------------------------------------------------------------------------------------------------

//CONTOUR CALCULATION
//----------------------------------------------------------------------------------------------------

void contour_center(Contour* cnt, float* x, float* y) {
	float sum_x = 0;
	float sum_y = 0;
	for (int i = 0; i < cnt->index; i++) {
		sum_x += cnt->points[i].x;
		sum_y += cnt->points[i].y;
	}
	*x = sum_x / cnt->index;
	*y = sum_y / cnt->index;
}

/*
return the extrem points of a contour
order N E S W
*/
struct point* get_contour_extreme(Contour* cnt) {
	struct point* result = calloc(4, sizeof(struct point));

	for (int i = 0; i < 4; i++) {
		result[i].x = cnt->points[0].x;
		result[i].y = cnt->points[1].y;
	}
	
	//remeber that y up --> down
	for (int i = 0; i < cnt->index; i++) {
		if (result[0].y > cnt->points[i].y) {//found N
			result[0].x = cnt->points[i].x;
			result[0].y = cnt->points[i].y;
		}

		if (result[1].x < cnt->points[i].x) {//found E
			result[1].x = cnt->points[i].x;
			result[1].y = cnt->points[i].y;
			
		}

		if (result[2].y < cnt->points[i].y) {//found S
			result[2].x = cnt->points[i].x;
			result[2].y = cnt->points[i].y;
		
		}

		if (result[3].x > cnt->points[i].x) {//found W
			result[3].x = cnt->points[i].x;
			result[3].y = cnt->points[i].y;
		
		}
	}

	return result;
}

/*
Compute area by counting how many pixels it has
it plots the contour on a buffer and then counts in between the contour

this function is NOT GUARANTEED to give the CORRECT area
this is because some contours have noises on their boundaries
however it gives very close result by applying a noise cleaner in the end

1 marks boundary
2 marks a pixel inside the boundary

this function uses the even/odd rule to determine if it is inside or outside the shape
it counts how many pixels are inside the boundary
then it cleans out from the other side pixels outside the boundary
*/
size_t get_contour_area(Contour* cnt) {
/*
	size_t* x;
	size_t* y;

	get_contour_extreme(cnt, x, y);

	size_t contour_width	= x[1] - x[3] +1;
	size_t contour_height 	= y[2] - y[0] +1;

	Image* buffer = new_image(1, contour_width, contour_height);

	if (buffer == NULL) {
		free(x);
		free(y);
		
		return 0;
	}
	
	for (int i = 0; i < cnt->index; i++)
		set_at(buffer, 0, cnt->points[i].x - x[3], cnt->points[i].y - y[0], 1);

	size_t area = 0;

	for (int i = 0; i < contour_height; i++) {
		value_t last	= 0;
		char 	count 	= 0;
		for (int j = 0; j < contour_width; j++) {
			value_t v = get_at(buffer, 0, j, i, 0);
			if (v == 1) { //count border
				area++;
				if (last == 0) //switch mode
					count = !count;
			}
			else {
				if (count) {
					area++;
					if (get_at(buffer, 0, j, i, 0) != 1)
						set_at(buffer, 0, j, i, 2);
				}
			}
			last = v;
		}
	}
	
	for (int i = 0; i < contour_height; i++) {
		for (int j = contour_width -1; j >= 0; j--) {
			if (get_at(buffer, 0, j, i, 0) == 1)
				break;
			else if (get_at(buffer, 0, j, i, 0) == 2) {
				area--;
				set_at(buffer, 0, j, i, 0);
			}	
		}
	}
	free_image(buffer);
	free(x);
	free(y);

	return area;
	*/
	return 0;
}

//----------------------------------------------------------------------------------------------------
