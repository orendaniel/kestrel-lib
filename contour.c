#include "contour.h"

#define IS_BORDER(IMAGE, X, Y) !(get_at(IMAGE, 0, X+1, Y, 0) && \
	get_at(IMAGE, 0, X, Y+1, 0) && \
	get_at(IMAGE, 0, X-1, Y, 0) && \
	get_at(IMAGE, 0, X, Y-1, 0))

int cmpfunc(const void* a, const void* b) {
	return *(size_t*)a - *(size_t*)b;
}	

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
			/*
			add these lines to enable 4 connectivity
			*/
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
			insert_point (cnt, nx_x, nx_y);
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
			set_at(buffer, 0, cnt->Xi[i], cnt->Yi[i], 0);// delete contour
		return NULL;
	}

}

Contour* new_contour() {
	size_t 	size 	= CONTOUR_ALLOCATION_SIZE;
	size_t* Xi		= calloc(size, sizeof(size_t));
	size_t* Yi		= calloc(size, sizeof(size_t));

	if (Xi != 0 && Yi != 0) {
		Contour* cnt = malloc(sizeof(Contour));

		cnt->Xi 	= Xi;
		cnt->Yi 	= Yi;
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
		cnt->Xi[cnt->index] = x;
		cnt->Yi[cnt->index] = y;
		cnt->index++;
	}
	else {
		size_t* tmp_x = realloc(cnt->Xi, sizeof(size_t) * (cnt->size + CONTOUR_ALLOCATION_SIZE));
		size_t* tmp_y = realloc(cnt->Yi, sizeof(size_t) * (cnt->size + CONTOUR_ALLOCATION_SIZE));
		if (tmp_x != 0 && tmp_y != 0) {
			cnt->Xi = tmp_x;
			cnt->Yi = tmp_y;
			cnt->size += CONTOUR_ALLOCATION_SIZE;
			cnt->Xi[cnt->index] = x;
			cnt->Yi[cnt->index] = y;
			cnt->index++;
		}
		else {
			fprintf(stderr, "Cannot resize contour\n");
		}
	}
}

void free_contour(Contour* cnt) {
	free(cnt->Xi);
	free(cnt->Yi);
	free(cnt);
}


/*
find the contours in a binary image using square trace with 8 connectivity
set steps to 1 for full precision at the price of speed.
recommended 3 steps for regular images
*/
Contour** find_contours(Image* img, size_t* index_size, size_t steps_x, size_t steps_y) {
	if (img->channels == 1) {
		Image* buffer = new_image(1, img->width, img->height); //keep track of points on other contours
		Contour** cnts = malloc(sizeof(Contour**));
		size_t amount = 0;
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
		return NULL;
	}
}

void contour_center(Contour* cnt, float* x, float* y) {
	float sum_x = 0;
	float sum_y = 0;
	for (int i = 0; i < cnt->index; i++) {
		sum_x += cnt->Xi[i];
		sum_y += cnt->Yi[i];
	}
	*x = sum_x / cnt->index;
	*y = sum_y / cnt->index;
}

/*
check if a point is inside the contour using raycast
and the even/odd rule
it is valid for positives only
*/

char is_inside_contour(Contour* cnt, float x, float y) {
	size_t* Xi 		= calloc(cnt->index, sizeof(size_t));
	size_t 	index 	= 0;
	if (Xi == NULL) {
		fprintf(stderr, "memory error\n");
		return 0;
	}

	for (int i = 0; i < cnt->index; i++) {
		if ((int)cnt->Xi[i] == (int)x && (int)cnt->Yi[i] == (int)y) {//point is on border
			free(Xi);
			return 1;
		}
		if ((int)cnt->Xi[i] > (int)x && (int)cnt->Yi[i] == (int)y) {
			Xi[index] = cnt->Xi[i];
			index++;
		}
	}
	
	qsort(Xi, index, sizeof(size_t), cmpfunc);

	if (index <= 1) {
		free(Xi);
		return index % 2 == 1;
	}
	if (index == 2) {
		free(Xi);
		return Xi[1] - Xi[0] < 2; //check if there is a significant gap
		//theoretically should have been < 1 however 2 was choose 
		//to ignore insignificant gaps
	}
	/*	
	this sections deals with multiple high pixels in a row
	count start from 1 and not 0 because there is a gap between nothing to Xi[0]
	*/
	int count = 1;
	for (int i = 0; i < index -1; i++){
		if (Xi[i +1] - Xi[i] > 2) {//check if there is a significant gap
			count += 1;
		}
	}
	free(Xi);
	return count % 2 == 1;
}


/*
return the extrem points of a contour
assumes 2 arrays with size 4
order N E S W
*/
void get_contour_extreme(Contour* cnt, size_t* x, size_t* y) {

	for (int i = 0; i < 4; i++){
		x[i] = cnt->Xi[0];
		y[i] = cnt->Yi[1];
	}
	
	//remeber that y up --> down
	for (int i = 0; i < cnt->index; i++){
		if (y[0] > cnt->Yi[i]) {//found N
			x[0] = cnt->Xi[i];
			y[0] = cnt->Yi[i];
		}

		if (x[1] < cnt->Xi[i]) {//found E
			x[1] = cnt->Xi[i];
			y[1] = cnt->Yi[i];
			
		}

		if (y[2] < cnt->Yi[i]) {//found S
			x[2] = cnt->Xi[i];
			y[2] = cnt->Yi[i];
		
		}

		if (x[3] > cnt->Xi[i]) {//found W
			x[3] = cnt->Xi[i];
			y[3] = cnt->Yi[i];
		
		}
	}
}

/*
Compute area by counting how many pixels it has
it plots the contour on a buffer and then counts in between the contour
*/
size_t get_contour_area(Contour* cnt) {
	size_t* x = calloc(4, sizeof(size_t));
	size_t* y = calloc(4, sizeof(size_t));

	get_contour_extreme(cnt, x, y);

	size_t contour_width	= x[1] - x[3]+1;
	size_t contour_height 	= y[2] - y[0]+1;

	Image* buffer = new_image(1, contour_width, contour_height);

	if (buffer == NULL)
		return 0;
	
	for (int i = 0; i < cnt->index; i++)
		set_at(buffer, 0, cnt->Xi[i] - x[3], cnt->Yi[i] - y[0], 1);

	size_t area = 0;
	for (int i = 0; i < contour_height; i++) {
		value_t last	= 0;
		char 	count 	= 0;
		for (int j = 0; j < contour_width; j++) {
			value_t v = get_at(buffer, 0, j, i, 0);
			if (v != 0) {
				area++;
				if (last == 0)
					count = !count;
			}
			else {
				if (count)
					area++;
			}
			last = v;
		}
	}
	free_image(buffer);
	free(x);
	free(y);

	return area;
}

