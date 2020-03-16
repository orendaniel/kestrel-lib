#include "image.h"
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

Image* new_image(size_t channels, size_t width, size_t height) {
	value_t* data = calloc(width * height * channels, sizeof(value_t));
	if (data) {
		Image* img = malloc(sizeof(Image));
		img->channels = channels;
		img->width = width;
		img->height = height;
		img->data = data;
		return img;
	}
	else {
		fprintf(stderr, "Cannot allocate image\n");
		return 0;
	}
}

value_t get_at(Image* img, size_t chnl, size_t x, size_t y, value_t def_value) {
	size_t index = chnl + img->channels * (x + y * img->width);
	if (index >= 0 && index < img->width * img->height * img->channels &&
			x < img->width && x >= 0 &&
			y < img->height && y >= 0 &&
			chnl < img->channels && chnl >= 0)

		return img->data[index];

	else {
		fprintf(stderr, "Out of array bounds\n");
		return def_value;
	}

}

void set_at(Image* img, size_t chnl, size_t x, size_t y, value_t value) {
	size_t index = chnl + img->channels * (x + y * img->width);
	if (index >= 0 && index < img->width * img->height * img->channels &&
			x < img->width && x >= 0 &&
			y < img->height && y >= 0 &&
			chnl < img->channels && chnl >= 0)

		img->data[index] = value;

	else
		fprintf(stderr, "Out of array bounds\n");
}


void free_image(Image* img) {
	free(img->data);
	free(img);
}


/*DO ERROR HANDLING FOR READ/WRITE*/
/*
MAKE A MACRO TO CALCULATE MAXIMUM SIZE
*/
void write_rgb_pixel_map(const char* file, Image* img) {
	FILE* f = fopen(file, "w");

	fprintf(f, "P3\n%d %d %d\n", img->width, img->height, 256);

	if (img->channels == 3) {
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				fprintf(f, "%d %d %d\n", 
					get_at(img, 0, j, i, -1), //red channel
					get_at(img, 1, j, i, -1), //blue channel
					get_at(img, 2, j, i, -1)); //green channel
			}
		}
	}
	else if (img->channels == 1) {
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				fprintf(f, "%d %d %d\n", 
					get_at(img, 0, j, i, -1), //red channel
					get_at(img, 0, j, i, -1), //blue channel
					get_at(img, 0, j, i, -1)); //green channel
			}
		}
	}
	else
		fprintf(stderr, "Must have only R G B channels or grayscale to write image\n");

	fclose(f);
}

/*
Currently ignore deviating formats and max value
*/
Image* read_rgb_pixel_map(const char* file) {
	FILE* f = fopen(file, "r");

	size_t width, height, max_value;
	
	fscanf(f, "%d %d %d", &width, &height, &max_value);
	Image* img = new_image(3, width, height);

	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			value_t r, b, g;
			fscanf(f, "%d %d %d", &r, &g, &b);
			set_at(img, 0, j, i, r), //red channel
			set_at(img, 1, j, i, g), //blue channel
			set_at(img, 2, j, i, b); //green channel
		}
	}

	fclose(f);
	return img;
}

Image* in_range(Image* img, value_t* lower, value_t* upper, value_t on, value_t off) {
	Image* result = new_image(1, img->width, img->height);
	size_t chnls = img->channels;
	value_t current[chnls];
	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			for (int c = 0; c < chnls; c++) 
				current[c] = get_at(img, c, j, i, 0);

			value_t is_on = on;
			for (int c = 0; c < chnls; c++) 
				if (current[c] < lower[c] || current[c] > upper[c])
					is_on = off;

			set_at(result, 0, j, i, is_on);
			
		}
	}
	return result;
}

Image* rgb_to_hsv(Image* img) {
	if (img->channels == 3) {
		Image* result = new_image(3, img->width, img->height);
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				float r 	= (float)get_at(img, 0, j, i, 0)/255;
				float g 	= (float)get_at(img, 1, j, i, 0)/255;
				float b		= (float)get_at(img, 2, j, i, 0)/255;
				float max 	= MAX(MAX(r, g), b);
				float min 	= MIN(MIN(r, g), b);
				float df	= max - min;
				if (df == 0) 
					set_at(result, 0, j, i, 0);
				else if (max == r)
					set_at(result, 0, j, i, (value_t)((int)(60 * ((g-b)/df) + 360) % 360));
				else if (max == g)
					set_at(result, 0, j, i, (value_t)((int)(60 * ((b-r)/df) + 120) % 360));
				else if (max == b)
					set_at(result, 0, j, i, (value_t)((int)(60 * ((r-g)/df) + 240) % 360));

				if (max == 0)
					set_at(result, 1, j, i, 0);
				else
					set_at(result, 1, j, i, (value_t)((df/max)*100));
					
				set_at(result, 2, j, i, (value_t)(max*100));
			}
		}
		
		return result;
	}
	else {
		fprintf(stderr, "Must have only R G B channels\n");
		return NULL;
	}
}
