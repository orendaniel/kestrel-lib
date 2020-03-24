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

#include "device.h"

/*
Communication with v4l
please refer to v4l and libv4l documentation
*/

//HELPERS
//----------------------------------------------------------------------------------------------------

static void	xioctl(int fh, int request, void* arg) {
	int r;

	do {
		r = v4l2_ioctl(fh, request, arg);
	} while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

	if (r == -1) 
		fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
}

void free_buffer(Buffer* bff) {
	free(bff->start);
	free(bff);
}

//----------------------------------------------------------------------------------------------------


//DEVICE FUNCTIONS
//----------------------------------------------------------------------------------------------------

Device*	new_device(const char* name, size_t width, size_t height) {
	Device* dev = calloc(1, sizeof(Device));

	dev->fd = v4l2_open(name, O_RDWR | O_NONBLOCK, 0);
	if (dev->fd < 0) {
		fprintf(stderr, "Unable to open device at %s\n", name);
		return NULL;
	}

	dev->fmt		= calloc(1, sizeof(struct v4l2_format));//allocate memory
	dev->v4l_buffer	= calloc(1, sizeof(struct v4l2_buffer));
	dev->req		= calloc(1, sizeof(struct v4l2_requestbuffers));
	dev->tv			= calloc(1, sizeof(struct timeval));

	if (!(dev->fmt && dev->v4l_buffer && dev->req && dev->tv)) {
		fprintf(stderr, "Cannot allocate device\n");
		exit(EXIT_FAILURE);
	}

	dev->fmt->type						= V4L2_BUF_TYPE_VIDEO_CAPTURE; //formats
	dev->fmt->fmt.pix.width 			= width;
	dev->fmt->fmt.pix.height			= height;
	dev->fmt->fmt.pix.pixelformat		= V4L2_PIX_FMT_RGB24;
	dev->fmt->fmt.pix.field				= V4L2_FIELD_INTERLACED;

	xioctl(dev->fd, VIDIOC_S_FMT, dev->fmt);

	if (dev->fmt->fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
		printf("Libv4l didn't accept RGB24 format\n");
		return NULL;
	}

	if ((dev->fmt->fmt.pix.width != width) || (dev->fmt->fmt.pix.height != height))
		printf("Warning: driver is sending image at %dx%d\n",
			dev->fmt->fmt.pix.width, dev->fmt->fmt.pix.height);


	dev->req->count		= 2;
	dev->req->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dev->req->memory	= V4L2_MEMORY_MMAP;

	xioctl(dev->fd, VIDIOC_REQBUFS, dev->req);

	dev->buffers = calloc(dev->req->count, sizeof(Buffer));

	for (dev->n_buffers = 0; dev->n_buffers < dev->req->count; ++dev->n_buffers) {
		dev->v4l_buffer->type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		dev->v4l_buffer->memory      = V4L2_MEMORY_MMAP;
		dev->v4l_buffer->index       = dev->n_buffers;

		xioctl(dev->fd, VIDIOC_QUERYBUF, dev->v4l_buffer);

		dev->buffers[dev->n_buffers].length = dev->v4l_buffer->length;
		dev->buffers[dev->n_buffers].start = v4l2_mmap(NULL, dev->v4l_buffer->length,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			dev->fd, dev->v4l_buffer->m.offset);

		if (MAP_FAILED == dev->buffers[dev->n_buffers].start) {
			perror("mmap");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < dev->n_buffers; ++i) {
		free(dev->v4l_buffer);
		dev->v4l_buffer	= calloc(1, sizeof(struct v4l2_buffer));

		dev->v4l_buffer->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		dev->v4l_buffer->memory = V4L2_MEMORY_MMAP;
		dev->v4l_buffer->index = i;
		xioctl(dev->fd, VIDIOC_QBUF, dev->v4l_buffer);
	}

	dev->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	xioctl(dev->fd, VIDIOC_STREAMON, &dev->type);	

	return dev;
}

Image* read_frame(Device* dev) {
	do {
		FD_ZERO(&dev->fds);
		FD_SET(dev->fd, &dev->fds);

		dev->tv->tv_sec	= 2;

		dev->tv->tv_usec = 0;

		dev->r = select(dev->fd + 1, &dev->fds, NULL, NULL, dev->tv);
	} while ((dev->r == -1 && (errno = EINTR)));

	if (dev->r == -1) {
		perror("select");
		return NULL;
	}


	free(dev->v4l_buffer);
	dev->v4l_buffer	= calloc(1, sizeof(struct v4l2_buffer));

	dev->v4l_buffer->type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	dev->v4l_buffer->memory = V4L2_MEMORY_MMAP;
	xioctl(dev->fd, VIDIOC_DQBUF, dev->v4l_buffer);

	Image* img = new_image(3, dev->fmt->fmt.pix.width, dev->fmt->fmt.pix.height);
	if (img) {
		memcpy(img->data, dev->buffers[dev->v4l_buffer->index].start, 
			img->channels * img->width * img->height);
		
		xioctl(dev->fd, VIDIOC_QBUF, dev->v4l_buffer);
		return img;
	}
	else {
		fprintf(stderr, "Cannot grab image from device\n");
		return NULL;
	}
}

void free_device(Device* dev) {
	dev->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(dev->fd, VIDIOC_STREAMOFF, &dev->type);
	for (int i = 0; i < dev->n_buffers; ++i) {
		v4l2_munmap(dev->buffers[i].start, dev->buffers[i].length);
	}
	v4l2_close(dev->fd);
	free(dev);
}

//----------------------------------------------------------------------------------------------------
