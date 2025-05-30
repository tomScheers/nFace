#include "camera.h"

#include <stdlib.h>

void writeImageData(BMPImage *image, const unsigned char *yuyvData,
                          const size_t dataSize, const size_t stride) {
  size_t size = 0;
  image->data = malloc(dataSize);

  for (size_t i = 0; i < (size_t)image->height; ++i) {
    const unsigned char *row =
        yuyvData + stride * (image->height - i - 1); // Adjust row pointer
    for (size_t j = 0; j < (size_t)image->width / 2; ++j) {
      int Y0 = row[j * 4 + 0];      // Y0
      int U = row[j * 4 + 1] - 128; // U0 (shared by two pixels)
      int Y1 = row[j * 4 + 2];      // Y1
      int V = row[j * 4 + 3] - 128; // V0 (shared by two pixels)

      // Convert first pixel (Y0, U, Y1)
      int R0 = Y0 + ((179 * V) >> 8);
      int G0 = Y0 - ((44 * U + 91 * V) >> 8);
      int B0 = Y0 + ((227 * U) >> 8);

      // Convert second pixel (Y1, U, V)
      int R1 = Y1 + ((179 * V) >> 8);
      int G1 = Y1 - ((44 * U + 91 * V) >> 8);
      int B1 = Y1 + ((227 * U) >> 8);

      // Ensure values are within range
      R0 = R0 < 0 ? 0 : (R0 > 255 ? 255 : R0);
      G0 = G0 < 0 ? 0 : (G0 > 255 ? 255 : G0);
      B0 = B0 < 0 ? 0 : (B0 > 255 ? 255 : B0);

      R1 = R1 < 0 ? 0 : (R1 > 255 ? 255 : R1);
      G1 = G1 < 0 ? 0 : (G1 > 255 ? 255 : G1);
      B1 = B1 < 0 ? 0 : (B1 > 255 ? 255 : B1);

      // Write the first pixel (BGR)
      unsigned char pixel0[3] = {B0, G0, R0};
      image->data[size++] = pixel0[0];
      image->data[size++] = pixel0[1];
      image->data[size++] = pixel0[2];

      // Write the second pixel (BGR)
      unsigned char pixel1[3] = {B1, G1, R1};
      image->data[size++] = pixel1[0];
      image->data[size++] = pixel1[1];
      image->data[size++] = pixel1[2];
    }
  }
}
