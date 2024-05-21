#ifndef FRAMESERVER_H
#define FRAMESERVER_H

#include "rgbimage.hpp"

#ifdef __linux__
#include "v4ldev.hpp"
#endif

#include "stream.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../ini.h"


namespace tilestreamer {

class frameserver_config {
  private:
    std::vector<stream *> streams;
    std::map<std::string, stream *> streams_by_name;
    int w, h, fps;
    uint16_t last_id;

    int httpport;
    std::string dev;
    std::string fontfilename;

  public:
    frameserver_config();
    void read_ini_config(std::string filename);
    int validate();
    void print() const;
    uint16_t get_last_id() {
      last_id += 1;
      return last_id - 1;
    };
    friend int inih_handler(
      void *configptr, const char *section,
      const char *name, const char *value
    );
    friend class frameserver;
};

class frameserver {
  private:
    std::vector<stream *> streams;

    frameserver_config fsc;
    
    rgbimage *frame;

  #ifdef __linux__
    v4ldev outdev;
  #endif

    int w, h, fps;
    volatile int keep_alive;

  public:
    frameserver();
    ~frameserver();

    stream *get_last_stream();

    void set_config(const frameserver_config &fsc);

    void start_streams();
    void stop_streams();

    int get_fps() { return fps; };
    void get_frame_as_jpeg(uint8_t *buffer, size_t *bufsize) const;
    rgbimage *get_frame() const { return frame; };
    void update_frame();

    friend class frameserver_config;
};

}
#endif // FRAMESERVER_H_
