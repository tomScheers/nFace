#ifndef CAM_H
#define CAM_H

#include "ita.h"

#include <stdio.h>
#include <ncurses.h>
#include <linux/videodev2.h>

int xioctl(int fh, int request, void *arg);

/*
 * @brief Opens the camera
 *
 * @details Opens the camera device at /dev/video0
 *
 * @return File decriptor for the camera device, or -1 on failure
 */
int openCamera();

/*
 * @brief Sets the format for the camera
 *
 * @details Sets the camera format to use YUYV, and what the image dimensions
 * should be
 *
 * @param cameraFd File descriptor of the camera
 * @param imageWidth The width of the image being captured
 * @param imageHeight The height of the image being captured
 * @param *p_fmt Format to set, this is used later in the code
 * @return -1 on failure
 */
int setFormat(const int cameraFd, const size_t imageWidth,
                     const size_t imageHeight, struct v4l2_format *p_fmt);

/*
 * @brief Request a camera buffer
 *
 * @details Request a buffer with data the camera captured
 *
 * @param cameraFd File descriptor for the camera
 * @return -1 on failure
 */
int requestBuffer(const int cameraFd);

/*
 * @brief Queues a buffer to the video device
 *
 * @details Places a previously requested and memory-mapped buffer into the
 * incoming queue to be filled with image data by the driver
 *
 * @param cameraFd File descriptor for camera
 * @return -1 on failure
 */
int queueBuffer(const int cameraFd);

/*
 * @brief Starts video stream
 *
 * @details Start streaming video to the queued buffer
 *
 * @param cameraFd File descriptor for the camera
 * @return -1 on failure
 *
 */
int startStream(const int cameraFd);

/*
 * @brief Queries buffer properties
 *
 * @details Populates infoBuf with the offset, length and other metadata about
 * the buffer previously requested via VIDEOC_REQBUFS
 *
 * @param cameraFd File descriptor for the camera to get the information of
 * @param infoBuff Buffer to write the device information to
 * @return -1 on failure
 */
int getDeviceInfo(const int cameraFd, struct v4l2_buffer *infoBuf);

/*
 * @brief Memory-maps a video capture buffer
 *
 * @details Uses mmap to map the buffer descirbed by buf into user psace. This allows direct acces to the image data
 *
 * @important Make sure to call unmap on the mapped memory!
 *
 * @param cameraFd File descriptor for the camera device
 * @param buf Buffer to map the memory to
 *
 * @return -1 on failure, 0 on success
 */
unsigned char *mapMemory(const int cameraFd,
                                const struct v4l2_buffer *buf);

/*
 * @brief Waits for a frame to be ready for capture
 *
 * @details Uses select() to block until the video device signal that a frame is available
 *
 * @param cameraFd File descriptor for the camera device
 * @return -1 on failure
 */
int selectFrame(const int cameraFd);

/*
 * @brief Dequeues buffer from the video device
 *
 * @details Retrieves a filled buffer from the outgoing queue after a frame has
 * been captured
 *
 * @param cameraFd File descriptor for camera
 * @return -1 on failure
 */
int dequeueBuf(const int cameraFd, const struct v4l2_buffer *buf);

/*
 * @brief ends video stream
 *
 * @param cameraFd File descriptor for camera
 * @return -1 on failure
 */
int endStream(const int cameraFd);

/*
 * @brief Generates a BITMAPINFOHEADER
 *
 * @details Constructs a 40-byte BMP info header with the given image width and height. This is part of the BMP file format
 * @param imageHeight Height of the image
 * @param imageWidth Width of the image
 */
unsigned char *getImageHeader(const size_t imageWidth,
                                     const size_t imageHeight);

/*
 * @brief Converts YUYV image data to BGR format and stores it ina BMPImage
 *
 * @details Take interleaved YUYV data, performs color conversion to BGR (as required by BMP), and writes it onto the BMPImage data field
 *
 * @param *image Pointer to the BMP image to write the data to
 * @param *yuyvData Actual image data to use, will be converted to BGR
 * @param dataSize Size in bytes of the yuyvData dataset
 * @param stride Bytes per line the camera has read
 */
void writeImageData(BMPImage *image, const unsigned char *yuyvData,
                          const size_t dataSize, const size_t stride);

#endif
