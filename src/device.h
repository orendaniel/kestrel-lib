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

#ifndef DEVICE_H
#define DEVICE_H
#include "common.h"
#include "image.h"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

typedef struct {
	void*	start;
	size_t	length;
} Buffer;

typedef struct {
	struct v4l2_format*				fmt;
	struct v4l2_buffer*				v4l_buffer;
	struct v4l2_requestbuffers*		req;
	struct timeval*					tv;
	enum v4l2_buf_type				type;
	fd_set							fds;
	int								r, fd;
	unsigned int					i, n_buffers;
	char							dev_name;
	char							out_name[256];
	Buffer*							buffers;
} Device;


// DEVICE FUNCTIONS
//----------------------------------------------------------------------------------------------------

Device*		new_device(const char* name, size_t width, size_t height);
Image*		read_frame(Device* dev);
void		free_device(Device* dev);

//----------------------------------------------------------------------------------------------------

#endif
