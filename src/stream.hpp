#ifndef SRC_STREAM_HPP_
#define SRC_STREAM_HPP_

#include <gst/gst.h>

#include <iostream>
#include <string>
#include <thread>
#include <sys/stat.h>

#include "rgbimage.hpp"

class stream {
 private:
  std::string name;
  std::string uri;
  std::string textoverlay;
  std::string videoflip;
  std::string fontfilename;

  // Output
  int x, y;
  int w, h;
  volatile int keep_alive;
  volatile int stop_signal;
  uint16_t id;
  uint16_t zindex;

  rgbimage *frame;
  rgbimage *overlaytext;

  // Gstreamer
  GstElement *pipeline;
  GstElement *appsink;
  GstBus *bus;

  std::string gst_pipeline_str;
  std::string gst_appsinkname;
  std::string gst_videoflip;

  std::thread streamthread;

 public:
  stream();
  stream(std::string uri);
  ~stream();
  void open();
  void start();
  // void stop();
  void close();
  void init_overlaytext();
  rgbimage *getRenderedOverlaytext();

  GstElement *getPipeline() { return pipeline; }
  void set_gst_pipeline_str(const std::string &s) { gst_pipeline_str = s; }
  void mkpipeline_s();
  rgbimage *getFrame() { return frame; }
  void setFrame(uint8_t *buffer, int w, int h);
  int get_id() { return id; }
  void set_id(int id) { this->id = id; }
  int get_zindex() { return zindex; }
  void set_zindex(int zindex) { this->zindex = zindex; }
  int get_x() { return x; }
  void set_x(int x) { this->x = x; }
  int get_y() { return y; }
  void set_y(int y) { this->y = y; }
  int get_w() { return w; }
  void set_w(int w) { this->w = w; }
  int get_h() { return h; }
  void set_h(int h) { this->h = h; }
  int get_keep_alive() { return keep_alive; }
  void set_uri(std::string uri) { this->uri = uri; }
  void set_textoverlay(std::string text) { textoverlay = text; }
  void set_name(std::string name) { this->name = name; }
  void set_fontfilename(std::string fontfilename) { this->fontfilename = fontfilename; }
  void setPos(int x, int y) {
    this->x = x;
    this->y = y;
  }
  void setDim(int w, int h) {
    this->w = w;
    this->h = h;
  }
  std::string get_name() { return name; }
  void print() {
    std::cout << "--------" << std::endl;
    std::cout << id << std::endl;
    std::cout << uri << std::endl;
    std::cout << textoverlay << std::endl;
    std::cout << w << "x" << h << std::endl;
    std::cout << x << "," << y << std::endl;
    std::cout << zindex << std::endl;
  }

  // Returns 0 when valid
  int validate() {
    int temp = 0;

    struct stat buffer;
    
    bool valid_uri = false;

    if (uri.rfind("http", 0) == 0) {
      valid_uri = true;
    }
    if (uri.rfind("rtsp", 0) == 0) {
      valid_uri = true;
    }
    if (!valid_uri) {
      if (::stat (uri.c_str(), &buffer) == 0) {
        valid_uri = true;
      } else {
        std::cout << "File not found:" << uri << std::endl;
      }
    }

    if (!valid_uri) {
      std::cout << "Invalid uri:" << uri << std::endl;
      temp ++;
    }


    if (w == INT_MIN) {
      std::cout << "w not defined for stream " << get_name() << std::endl;
      temp++;
    }
    if (h == INT_MIN) {
      std::cout << "h not defined for stream " << get_name() << std::endl;
      temp++;
    }
    if (x == INT_MIN) {
      std::cout << "x not defined for stream " << get_name() << std::endl;
      temp++;
    }
    if (y < 0) {
      std::cout << "y not defined for stream " << get_name() << std::endl;
      temp++;
    }
    return temp;
  }
};

#endif  // SRC_STREAM_HPP_
