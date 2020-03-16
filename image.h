#ifndef IMAGE_H
#define IMAGE_H

#include "common.h"

typedef struct {
	size_t 	channels;
	size_t 	width;
	size_t 	height;
	value_t *data;
} Image;

Image*		new_image(size_t channels, size_t width, size_t height);
value_t 	get_at(Image *img, size_t chnl, size_t x, size_t y, value_t def_value);
void		set_at(Image *img, size_t chnl, size_t x, size_t y, value_t value);
void		free_image(Image *img);
void		write_rgb_pixel_map(const char *file, Image *img);
Image* 		read_rgb_pixel_map(const char *file);
Image* 		in_range(Image* img, value_t* lower, value_t* upper, value_t on, value_t off);
Image* 		rgb_to_hsv(Image* img);


#endif
