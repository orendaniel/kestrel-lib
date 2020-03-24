/*
Kestrel vision library
Copyright (C) 2020  Oren Daniel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define CONTOUR_ALLOCATION_SIZE 100

#define DEFAULT_DEVICE_WIDTH 	160
#define DEFAULT_DEVICE_HEIGHT 	120

#define DEFAULT_STEPS_TRACING 	3

#define MAX_VALUE UCHAR_MAX

typedef unsigned char value_t;

#endif
