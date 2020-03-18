#include "contour.h"

#define IS_BORDER(IMAGE, X, Y) !(get_at(IMAGE, 0, X+1, Y, 0) && \
	get_at(IMAGE, 0, X, Y+1, 0) && \
	get_at(IMAGE, 0, X-1, Y, 0) && \
	get_at(IMAGE, 0, X, Y-1, 0))
			
Contour* square_trace(size_t st_x, size_t st_y, Image* img, Image* buffer) {
	Contour* cnt = new_contour();
	insert_point(cnt, st_x, st_y);
	size_t next_step_x = 0;//go left
	size_t next_step_y = -1;

	size_t nx_x = st_x + next_step_x;
	size_t nx_y = st_y + next_step_y;
	
	/*
	counter should not exceed 8
	because a square has 8 sides
	*/
	int safty_counter = 0;


	while (!(nx_x == st_x && nx_y == st_y) && safty_counter < 8) {
		if (get_at(img, 0, nx_x, nx_y, 0) == 0) {
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
			set_at (buffer, 0, nx_x, nx_y, 255);

			size_t tmp 	= next_step_x;
			next_step_x = next_step_y;//go left
			next_step_y = -tmp;

			nx_x += next_step_x;
			nx_y += next_step_y;
			
			safty_counter = 0;	
		}
	
	}
	return cnt;

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
			cnt->Xi = tmp_x;//check if need to free Xi before
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
find the contours in a binary image using square trace with 4 connectivity
set steps to 1 for full precision at the price of speed.
recommended 3 steps for regular images
*/
Contour** find_contours(Image* img, size_t* index_size, size_t steps_x, size_t steps_y) {
	if (img->channels == 1) {
		Image* buffer = new_image(1, img->width, img->height);
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
