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

#include "image.h"

#define MIN(MIN_A,MIN_B) (((MIN_A)<(MIN_B))?(MIN_A):(MIN_B))
#define MAX(MAX_A,MAX_B) (((MAX_A)>(MAX_B))?(MAX_A):(MAX_B))

// HELPERS
//----------------------------------------------------------------------------------------------------

static int add(value_t current, float x) {
	return (int)((float)current + x);
}

static int subtract(value_t current, float x) {
	return (int)((float)current - x);
}


static int multiply(value_t current, float x) {
	return (int)((float)current * x);
}

static int divide(value_t current, float x) {
	return (int)((float)current / x);
}

static Image* arth_operation(Image* img, int (*fn)(value_t value, float x), float x) {
	Image* result = make_image(img->channels, img->width, img->height);

	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			for (int c = 0; c < img->channels; c++){
				value_t v = get_at(img, c, j, i, 0);
				int nv = (*fn)(v, x);

				if (nv < 0) 
					nv = 0;

				else if (nv > MAX_VALUE) 
					nv = MAX_VALUE;

				set_at(result, c, j, i, (value_t)nv);
			}
		}
	}

	return result;
}

static value_t and(value_t a, value_t b) {
	return a && b;
}

static value_t or(value_t a, value_t b) {
	return a || b;
}

static value_t xor(value_t a, value_t b) {
	return (!a && b) || (a && !b);
}

static Image* logic_operation(Image* img1, Image* img2, value_t (*fn)(value_t a, value_t b)) {
	if (img1->channels == 1 && img2->channels == 1) {
		size_t width 	= MAX(img1->width, img2->width);	
		size_t height 	= MAX(img1->height, img2->height);	
		Image* result 	= make_image(1, width, height);

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++)
				set_at(result, 0, i, j, (*fn)(get_at(img1, 0, j, i, 0), get_at(img2, 0, j, i, 0)));
		}
		return result;
	}
	else {
		fprintf(stderr, "Bitwise operation for binary images only\n");
		return NULL;
	}
}


static double sobel_helper(Image* img, size_t x, size_t y) {
	int Gx[3][3] = {{-1, 0, 1},
					{-2, 0, 2},
					{-1, 0, 1}} ;

	int Gy[3][3] = {{-1, -2, -1},
					{0, 0, 0},
					{1, 2, 1}};
	double mag_x = 0, mag_y = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			mag_x += Gx[i][j] * get_at(img, 0, x - j, y - i, 0);
			mag_y += Gy[i][j] * get_at(img, 0, x - j, y - i, 0);
		}
	}
	return pow(mag_x * mag_x + mag_y * mag_y, 0.5);
}
//----------------------------------------------------------------------------------------------------

// COMMON FUNCTIONS
//----------------------------------------------------------------------------------------------------

Image* make_image(size_t channels, size_t width, size_t height) {
	value_t* data = calloc(width * height * channels, sizeof(value_t));
	if (data) {
		Image* img 		= malloc(sizeof(Image));
		img->channels 	= channels;
		img->width 		= width;
		img->height 	= height;
		img->data 		= data;
		return img;
	}
	else {
		fprintf(stderr, "Cannot allocate image\n");
		exit(EXIT_FAILURE);
	}
}

void free_image(Image* img) {
	free(img->data);
	free(img);
}

/*
get and set return or set the value at (channel, x, y) for Image img.

get and set don't return error if given invalid index
that is because some algorithms can go out of range

get_at will just return a default value if out of range
*/
value_t get_at(Image* img, size_t chnl, size_t x, size_t y, value_t def_value) {
	size_t index = chnl + img->channels * (x + y * img->width);
	if (index >= 0 && index < img->width * img->height * img->channels &&
			x < img->width && x >= 0 &&
			y < img->height && y >= 0 &&
			chnl < img->channels && chnl >= 0)

		return img->data[index];

	else 
		return def_value;

}

void set_at(Image* img, size_t chnl, size_t x, size_t y, value_t value) {
	size_t index = chnl + img->channels * (x + y * img->width);
	if (index >= 0 && index < img->width * img->height * img->channels &&
			x < img->width && x >= 0 &&
			y < img->height && y >= 0 &&
			chnl < img->channels && chnl >= 0)

		img->data[index] = value;
}

Image* split_channel(Image* img, size_t c) {
	if (c < img->channels) {
		Image* result = make_image(1, img->width, img->height);
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) 
				set_at(result, 0, j, i, get_at(img, c, j, i, 0));
		}
		return result;
	}
	else {
		fprintf(stderr, "Channel given does not exsits in image\n");
		return NULL;
	}
}

/*
in range will return a binary image if pixel values are in range

in range doesn't check if lower and upper's array lengths are valid
it assumes that lower and upper's length are both equal to the number of channels
*/
Image* in_range(Image* img, value_t* lower, value_t* upper, value_t on, value_t off) {
	Image* result = make_image(1, img->width, img->height);
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

/*
RGB to HSV conversion
*/
Image* rgb_to_hsv(Image* img) {
	if (img->channels == 3) {
		Image* result = make_image(3, img->width, img->height);
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

/*
any image type to grayscale conversion
average of all channels
*/
Image* grayscale(Image* img) {
	Image* result 		= make_image(1, img->width, img->height);
	size_t chnls_amount = img->channels;

	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			int sum = 0;
			for (int c = 0; c < chnls_amount; c++)
				sum += get_at(img, c, j, i, 0);
				
			set_at(result, 0, j, i, sum/chnls_amount);
		}
	}
	
	return result;
}

/*
grayscale to sobel
*/
Image* sobel(Image* img) {

	if (img->channels == 1) {

		Image* result = make_image(1, img->width, img->height);
		for (int i = 2; i < img->height -2; i++) {
			for (int j = 2; j < img->width -2; j++) {
				int 	sv 	= (int)sobel_helper(img, j, i);
				value_t v 	= 0;
				if (sv > MAX_VALUE)
					v = MAX_VALUE;
				else
					v = sv;

				set_at(result, 0, j, i, v);
			}
		}

		return result;
	}
	else {
		fprintf(stderr, "Sobel operator for grayscale images only\n");
		return NULL;
	}
}

/*
returns the inverted image
*/
Image* invert_image(Image* img) {
	Image* result = make_image(img->channels, img->width, img->height);

	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			for (int c = 0; c < img->channels; c++)
				set_at(result, c, j, i, MAX_VALUE - get_at(img, c, j, i, 0));
		}
	}
	
	return result;
}

//----------------------------------------------------------------------------------------------------

// I/O FUNCTIONS
//----------------------------------------------------------------------------------------------------

/*
functions to read and write .ppm files
*/
void write_pixel_map(const char* file, Image* img) {
	FILE* f = fopen(file, "w");

	if (f == NULL) {
		fprintf(stderr, "Cannot open file\n");
		return;
	}


	if (img->channels == 3) { // RGB image
		fprintf(f, "P3\n%ld %ld %d\n", img->width, img->height, MAX_VALUE);
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				fprintf(f, "%d %d %d\n", 
					get_at(img, 0, j, i, 0), // red channel
					get_at(img, 1, j, i, 0), // green channel
					get_at(img, 2, j, i, 0)); // blue channel
			}
		}
	}

	else if (img->channels == 1) { // grayscale image
		fprintf(f, "P2\n%ld %ld %d\n", img->width, img->height, MAX_VALUE);
		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++) {
				fprintf(f, "%d\n", get_at(img, 0, j, i, 0));
			}
		}
	}
	else
		fprintf(stderr, "Must have only R G B channels or grayscale to write image\n");

	fclose(f);
}

/*
doesn't accept comments
*/
Image* read_pixel_map(const char* file) {
	FILE* f = fopen(file, "r");

	size_t width, height, max_value;
	char gray_mode = 0;

	Image* result;

	char buffer[3];

	if (f == NULL) {
		fprintf(stderr, "Cannot open file\n");
		return NULL;
	}

	if (!fgets(buffer, sizeof(buffer), f)) {
		fprintf(stderr, "Cannot read file\n");
		fclose(f);
		return NULL;
	}

	if (buffer[0] == 'P' && buffer[1] == '3')
		gray_mode = 0;
	else if (buffer[0] == 'P' && buffer[1] == '2')
		gray_mode = 1;
	else {
		fprintf(stderr, "Invalid PPM format\n");
		fclose(f);
		return NULL;
	}
	
	fscanf(f, "%ld %ld %ld", &width, &height, &max_value);


	if (gray_mode) {
		result = make_image(1, width, height);
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				value_t v;
				if (fscanf(f, "%hhd", &v))
					set_at(result, 0, j, i, v);
			}
		}
	}
	else {
		result = make_image(3, width, height);
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				for (int c = 0; c < 3; c++) {
					value_t v;
					if (fscanf(f, "%hhd", &v))
						set_at(result, c, j, i, v);
				}
			}
		}
	}

	fclose(f);
	return result;
}
//----------------------------------------------------------------------------------------------------

// IMAGE OPERATORS
//----------------------------------------------------------------------------------------------------

char image_equality(Image* img1, Image* img2) {
	if (img1->channels == img2->channels &&
		img1->width == img2->width &&
		img1->height == img2->height) {
		
		for (int i = 0; i < img1->height; i++) {
			for (int j = 0; j < img1->height; j++) {
				for (int c = 0; c < img1->channels; c++) 
					if (get_at(img1, c, j, i, 0) != get_at(img2, c, j, i, 0))
						return 0;
			}
		}
		return 1;
	}
	else
		return 0;

}

Image* concat_channels(Image* img1, Image* img2) {
	size_t max_width 	= MAX(img1->width, img2->width);
	size_t max_height 	= MAX(img1->height, img2->height);
	size_t channels 	= img1->channels + img2->channels;
	Image* result 		= make_image(channels, max_width, max_height);

	for (int i = 0; i < max_height; i++) {
		for (int j = 0; j < max_width; j++){
			for (int c = 0; c < img1->channels; c++) 
				set_at(result, c, j, i, get_at(img1, c, j, i, 0));

			for (int c = 0; c < img2->channels; c++) 
				set_at(result, img1->channels + c, j, i, get_at(img2, c, j, i, 0));

		}
	}

	return result;
}


Image* image_add(Image* img, float x) {
	return arth_operation(img, &add, x);
}

Image* image_sub(Image* img, float x) {
	return arth_operation(img, &subtract, x);
}

Image* image_mul(Image* img, float x) {
	return arth_operation(img, &multiply, x);
}

Image* image_div(Image* img, float x) {
	return arth_operation(img, &divide, x);
}


Image* image_not(Image* img) {
	if (img->channels == 1) {
		Image* result = make_image(img->channels, img->width, img->height);

		for (int i = 0; i < img->height; i++) {
			for (int j = 0; j < img->width; j++)
				set_at(result, 0, j, i, !get_at(img, 0, j, i, 0));
		}
		return result;
	}
	else {
		fprintf(stderr, "Bitwise operation for binary images only\n");
		return NULL;
	}
}

Image* image_and(Image* img1, Image* img2) {
	return logic_operation(img1, img2, &and);
}

Image* image_or(Image* img1, Image* img2) {
	return logic_operation(img1, img2, &or);
}

Image* image_xor(Image* img1, Image* img2) {
	return logic_operation(img1, img2, &xor);
}

//----------------------------------------------------------------------------------------------------
