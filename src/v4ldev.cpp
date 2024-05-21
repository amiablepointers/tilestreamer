#ifdef __linux__

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/module.h>
#include <linux/version.h>
#include <linux/videodev2.h>

#include "rgbimage.hpp"
#include "v4ldev.hpp"

v4ldev::v4ldev() {
  fd = 0;
  isopen = false;
}

bool v4ldev::is_open() { 
  return (isopen); 
}

v4ldev::v4ldev(std::string dev, uint16_t w, uint16_t h) {
  this->dev = dev;
  this->w = w;
  this->h = h;
  fd = 0;
  isopen = false;
}

void v4ldev::close() {
  ::close(fd);
  isopen = false;
}

void v4ldev::open(std::string dev, uint16_t w, uint16_t h) {
  // /sbin/modprobe v4l2loopback video_nr=3,4,7

  struct v4l2_format vfmt;
  int ret;

  this->dev = dev;
  this->w = w;
  this->h = h;
  fd = 0;

  fd = ::open(dev.c_str(), O_RDWR);

  memset(&vfmt, 0, sizeof(vfmt));
  vfmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

  ret = ioctl(fd, VIDIOC_G_FMT, &vfmt);
  if (ret < 0) {
    printf("V4L: Set camera frame format error on %s\n", dev.c_str());
    // exit(EXIT_FAILURE);
  }

  vfmt.fmt.pix.width = w;
  vfmt.fmt.pix.height = h;

  vfmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;

  ret = ioctl(fd, VIDIOC_S_FMT, &vfmt);
  if (ret < 0) {
    printf("V4L: Set camera frame format error on %s\n", dev.c_str());
    // exit(EXIT_FAILURE);
  }

  if (ret >= 0) {
    isopen = true;
  }
}

void v4ldev::write_frame(rgbimage *img) {
  if (isopen) {
    write(fd, img->getBuffer(), 3 * w * h);
  }
}

void v4ldev::write_jpeg_frame(uint8_t *buffer, size_t length) {
  if (isopen) {
    write(fd, buffer, length);
  }
}

#endif
