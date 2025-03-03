#ifndef CAM_H
#define CAM_H

#include <linux/videodev2.h>
#include <stdlib.h>
#define FPS 20


void captureFrame(size_t img_width, size_t img_height);

#endif
