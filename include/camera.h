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
int open_camera();

/*
 * @brief Sets the format for the camera
 *
 * @details Sets the camera format to use YUYV, and what the image dimensions
 * should be
 *
 * @param camera_fd File descriptor of the camera
 * @param image_width The width of the image being captured
 * @param image_height The height of the image being captured
 * @param *p_fmt Format to set, this is used later in the code
 * @return -1 on failure
 */
int set_format(int camera_fd, size_t image_width,
                     size_t image_height, struct v4l2_format *p_fmt);

/* @brief Request a camera buffer
 *
 * @details Request a buffer with data the camera captured
 *
 * @param camera_fd File descriptor for the camera
 * @return -1 on failure
 */
int request_buf(int camera_fd);

/*
 * @brief Queues a buffer to the video device
 *
 * @details Places a previously requested and memory-mapped buffer into the
 * incoming queue to be filled with image data by the driver
 *
 * @param camera_fd File descriptor for camera
 * @return -1 on failure
 */
int queue_buf(int camera_fd);

/*
 * @brief Starts video stream
 *
 * @details Start streaming video to the queued buffer
 *
 * @param camera_fd File descriptor for the camera
 * @return -1 on failure
 *
 */
int start_stream(int camera_fd);

/*
 * @brief Queries buffer properties
 *
 * @details Populates infoBuf with the offset, length and other metadata about
 * the buffer previously requested via VIDEOC_REQBUFS
 *
 * @param camera_fd File descriptor for the camera to get the information of
 * @param info_buf Buffer to write the device information to
 * @return -1 on failure
 */
int get_device_info(int camera_fd, struct v4l2_buffer *info_buf);

/*
 * @brief Memory-maps a video capture buffer
 *
 * @details Uses mmap to map the buffer descirbed by buf into user psace. This allows direct acces to the image data
 *
 * @important Make sure to call unmap on the mapped memory!
 *
 * @param camera_fd File descriptor for the camera device
 * @param buf Buffer to map the memory to
 *
 * @return -1 on failure, 0 on success
 */
unsigned char *map_memory(int camera_fd,
                                const struct v4l2_buffer *buf);

/*
 * @brief Waits for a frame to be ready for capture
 *
 * @details Uses select() to block until the video device signal that a frame is available
 *
 * @param camera_fd File descriptor for the camera device
 * @return -1 on failure
 */
int select_frame(int camera_fd);

/*
 * @brief Dequeues buffer from the video device
 *
 * @details Retrieves a filled buffer from the outgoing queue after a frame has
 * been captured
 *
 * @param camera_fd File descriptor for camera
 * @return -1 on failure
 */
int dequeue_buf(int camera_fd, struct v4l2_buffer *buf);

/*
 * @brief ends video stream
 *
 * @param camera_fd File descriptor for camera
 * @return -1 on failure
 */
int end_stream(int camera_fd);

/*
 * @brief Generates a BITMAPINFOHEADER
 *
 * @details Constructs a 40-byte BMP info header with the given image width and height. This is part of the BMP file format
 * @param image_height Height of the image
 * @param image_width width of the image
 */
unsigned char *get_image_header(size_t image_width,
                                     size_t image_height);

/*
 * @brief Converts YUYV image data to BGR format and stores it ina BMPImage
 *
 * @details Take interleaved YUYV data, performs color conversion to BGR (as required by BMP), and writes it onto the BMPImage data field
 *
 * @param *image Pointer to the BMP image to write the data to
 * @param *yuyv_data Actual image data to use, will be converted to BGR
 * @param data_size Size in bytes of the yuyvData dataset
 * @param stride Bytes per line the camera has read
 */
void write_image_data(BMP_image *image, const unsigned char *yuyv_data,
                          size_t data_size, size_t stride);

#endif
