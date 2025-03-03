#include "camera.h"
#include "convert_bmp.h"
#include "ita.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main() {
  size_t img_width = 320, img_height = 280;
  size_t blockWidthToHeightRatio =
      2; // The ratio of a chars width in the terminal to it's height
  captureFrame(img_width, img_height);
  fprintf(stdout, "Image captured!\n");
  BMPImg *img = loadBMP("output/output.bmp");
  fprintf(stdout, "%d X %d\n", img->width, img->height);
  float ratio = (float)((float)img->width / (float)img->height);
  fprintf(stdout, "1");
  int factor = 200;
  fprintf(stdout, "2");
  int renderWidth = factor * ratio;
  fprintf(stdout, "1");
  int renderHeight = factor / blockWidthToHeightRatio;
  fprintf(stdout, "1");
  fprintf(stdout, "Render dimensions: %d X %d\n", renderWidth, renderHeight);
  assert(renderWidth <= img_width && renderHeight <= img_height);
  renderASCII(img, renderWidth, renderHeight);
  fprintf(stdout, "1");
  free(img);

  return 0;
}
