#ifndef CAM_H
#define CAM_H

#include "ita.h"
#include <linux/videodev2.h>
#include <stdlib.h>
#include <stdio.h>
#define FPS 20


//int xioctl(int fh, int request, void *arg);
//BMPImg* captureFrame(int fd, struct v4l2_format fmt, size_t img_width, size_t img_height);
void startCamera(size_t imageWidth, size_t imageHeight);

#endif
