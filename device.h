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


//DEVICE FUNCTIONS
//----------------------------------------------------------------------------------------------------

Device*		new_device(const char* name, size_t width, size_t height);
Image*		read_frame(Device* dev);
void		free_device(Device* dev);

//----------------------------------------------------------------------------------------------------

#endif
