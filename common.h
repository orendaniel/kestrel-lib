#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define CONTOUR_ALLOCATION_SIZE 100

#define DEFAULT_DEVICE_WIDTH 	160
#define DEFAULT_DEVICE_HEIGHT 	120

#define DEFAULT_STEPS_TRACING 	3


typedef unsigned char value_t;

#endif
