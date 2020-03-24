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

struct stack* new_stack(size_t max) {
	struct stack* stk = calloc(1, sizeof(struct stack*));

	stk->max 	= max;
	stk->size 	= 0;
	stk->items 	= calloc(max, sizeof(struct point));

	return stk;
}

void stack_push(struct stack* stk, struct point p) {
	if (stk->size != stk->max - 1) 
		stk->items[++stk->size] = p;
	else {
		struct point* tmp = realloc(stk->items, (stk->max +100) * sizeof(struct point));
		if (tmp != 0) {
			stk->items = tmp;
			stk->max += 100;
			stk->items[++stk->size] = p;

		}
		else
			fprintf(stderr, "stack overflow\n");
	}
}

struct point stack_pop(struct stack* stk) {
	if (stk->size > 0)
		return stk->items[stk->size--];
	else {
		fprintf(stderr, "stack underflow\n");
		struct point p = {0, 0};
		return p;
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

		for (int i = 0; i < img->height; i += steps_y) {
			for (int j = 0; j < img->width; j += steps_x) {
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
Calculate area by plotting contour and using flood fill 
to count area outside the contour and subtracting it
from the area of the buffer
*/
size_t get_contour_area(Contour* cnt) {
	struct point* exp = get_contour_extreme(cnt);

	size_t cnt_w = exp[1].x - exp[3].x +1;
	size_t cnt_h = exp[2].y - exp[0].y +1;

	Image* buffer = new_image(1, cnt_w +2, cnt_h +2);

	size_t area = (cnt_w +2) * (cnt_h +2);

	for (int i = 0; i < cnt->index; i++)
		set_at(buffer, 0, cnt->points[i].x -exp[3].x +1, cnt->points[i].y -exp[0].y +1, 1);

	struct stack* 	stk = new_stack(area);
	struct point 	p 	= {0, 0};
	stack_push(stk, p);

	while (stk->size > 0) {
		struct point p = stack_pop(stk);
		
		size_t x = p.x;
		size_t y = p.y;

		if (get_at(buffer, 0, x, y, 1) == 0) {
			set_at(buffer, 0, x, y, 2);
			area--;
			struct point p1 = {x +1, y};
			struct point p2 = {x -1, y};
			struct point p3 = {x, y +1};
			struct point p4 = {x, y -1};
			stack_push(stk, p1);
			stack_push(stk, p2);
			stack_push(stk, p3);
			stack_push(stk, p4);
		
		} 
	}
	
	free(stk->items);
	free(stk);
	free(buffer);
	free(exp);
	return area;
}

void fit_line(Contour* cnt, float* m, float* b) {
	float sum_x 	= 0;
	float sum_y 	= 0; 
	float sum_xy 	= 0;
	float sum_x2 	= 0; 

	for (int i = 0; i < cnt->index; i++) { 
		sum_x 	+= cnt->points[i].x; 
		sum_y 	+= cnt->points[i].y; 
		sum_xy 	+= cnt->points[i].x * cnt->points[i].y; 
		sum_x2 	+= (cnt->points[i].x * cnt->points[i].x); 
	} 

	*m = (cnt->index * sum_xy - sum_x * sum_y) / (cnt->index * sum_x2 - (sum_x * sum_x)); 
	*b = (sum_y - (*m) * sum_x) / cnt->index; 

	
}

//----------------------------------------------------------------------------------------------------
