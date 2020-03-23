#ifndef IMAGE_H
#define IMAGE_H

#include "common.h"

typedef struct {
	size_t		channels;
	size_t		width;
	size_t		height;
	value_t*	data;
} Image;

//COMMON FUNCTIONS
//----------------------------------------------------------------------------------------------------

Image* 		new_image(size_t channels, size_t width, size_t height);
void 		free_image(Image* img);
value_t 	get_at(Image* img, size_t chnl, size_t x, size_t y, value_t def_value);
void 		set_at(Image* img, size_t chnl, size_t x, size_t y, value_t value);
Image* 		split_channel(Image* img, size_t c);
Image* 		in_range(Image* img, value_t* lower, value_t* upper, value_t on, value_t off);
Image* 		rgb_to_hsv(Image* img);
Image* 		grayscale(Image* img);
Image* 		invert_image(Image* img);

//----------------------------------------------------------------------------------------------------

//IO
//----------------------------------------------------------------------------------------------------

void 	write_pixel_map(const char* file, Image* img);
Image* 	read_pixel_map(const char* file);

//----------------------------------------------------------------------------------------------------

//IMAGE OPERATORS
//----------------------------------------------------------------------------------------------------

char 	image_equality(Image* img1, Image* img2);
Image* 	concat_channels(Image* img1, Image* img2);
Image* 	image_add(Image* img, float x);
Image* 	image_sub(Image* img, float x);
Image* 	image_mul(Image* img, float x);
Image* 	image_div(Image* img, float x);
Image* 	image_not(Image* img);
Image* 	image_and(Image* img1, Image* img2);
Image* 	image_or(Image* img1, Image* img2);
Image* 	image_xor(Image* img1, Image* img2);

//----------------------------------------------------------------------------------------------------


#endif
