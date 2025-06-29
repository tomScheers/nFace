#include "camera.h"

#include <fcntl.h>

int open_camera(void) { return open("/dev/video0", O_RDWR); }
