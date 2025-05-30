#include "camera.h"

#include <fcntl.h>

int openCamera() { return open("/dev/video0", O_RDWR); }
