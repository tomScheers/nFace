#include "camera.h"
#include <stdlib.h>
#include <string.h>

#define HEADER_SIZE 40

unsigned char *get_image_header(size_t image_width,
                                     size_t image_height) {
  unsigned char *info_header =
     malloc(HEADER_SIZE * sizeof(unsigned char));
  unsigned char temp[HEADER_SIZE] = {
      40, 0, 0, 0, // Header size
      0,  0, 0, 0, // Image width
      0,  0, 0, 0, // Image height
      1,  0,       // biPlanes
      24, 0, 0, 0, // 24 bits per pixel
      0,  0, 0, 0, // Compression method (none)
      0,  0, 0, 0, // biSizeImage
      0,  0, 0, 0, // Printing related
      0,  0, 0, 0, // Printing related
      0,  0, 0, 0, // Color importancy
  };

  memcpy(info_header, temp, sizeof(temp));

  info_header[4] = (unsigned char)(image_width);
  info_header[5] = (unsigned char)(image_width >> 8);
  info_header[6] = (unsigned char)(image_width >> 16);
  info_header[7] = (unsigned char)(image_width >> 24);

  info_header[8] = (unsigned char)(image_height);
  info_header[9] = (unsigned char)(image_height >> 8);
  info_header[10] = (unsigned char)(image_height >> 16);
  info_header[11] = (unsigned char)(image_height >> 24);

  size_t row_size = (image_width * 3 + 3) & ~3;
  size_t image_size = row_size * image_height;
  info_header[20] = (unsigned char)(image_size);
  info_header[21] = (unsigned char)(image_size >> 8);
  info_header[22] = (unsigned char)(image_size >> 16);
  info_header[23] = (unsigned char)(image_size >> 24);
  return info_header;
}
