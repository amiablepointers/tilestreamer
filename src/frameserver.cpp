#include "frameserver.hpp"
#include <array>
#include <functional>
#include <unistd.h>

namespace tilestreamer {

frameserver::frameserver() {
  fps = 10;
  keep_alive = 1;
  frame = NULL;
}

frameserver::~frameserver() {
  for (int i = 0; i < (int)streams.size(); i++) {
    streams[i]->close();
    delete streams[i];
    streams[i] = NULL;
  }
  delete (frame);
  frame = NULL;
}

int frameserver_config::validate() {
  int temp = 0;

  if (w <= 0) {
    temp++;
  }
  if (h <= 0) {
    temp++;
  }
  if (fps <= 0) {
    temp++;
  }
  for (int i = 0; i < (int)streams.size(); i++) {
    temp += streams[i]->validate();
  }
  return temp;
}

void frameserver_config::print() const {
  std::cout << dev << std::endl;
  std::cout << w << "x" << h << "@" << fps << " fps" << std::endl;
  std::cout << streams.size() << " streams" << std::endl;

  for (int i = 0; i < (int)streams.size(); i++) {
    streams[i]->print();
  }
}

void frameserver::start_streams() {
  frame = new rgbimage(w, h);

  for (int i = 0; i < (int)streams.size(); i++) {
    streams[i]->open();
  }
}

void frameserver::stop_streams() {
  keep_alive = 0;
  for (int i = 0; i < (int)streams.size(); i++) {
    std::cout << "Stopping " << streams[i]->get_name() << std::endl;
    streams[i]->close();
  }
  delete (frame);
  frame = NULL;
}

void frameserver::update_frame() {
  if (frame != NULL) {
    for (int i = 0; i < (int)streams.size(); i++) {
      frame->paste(streams[i]->getFrame(), streams[i]->get_x(),
      streams[i]->get_y());
    }
  }
}

void frameserver::get_frame_as_jpeg(uint8_t *buffer, size_t *bufsize) const {
  frame->write_jpeg_mem(&buffer, bufsize);
}

void frameserver::set_config(const frameserver_config &fsc) {
  for (int i = 0; i < (int)fsc.streams.size(); i++) {
    streams.push_back(fsc.streams[i]);
  }
  fps = fsc.fps;
  w = fsc.w;
  h = fsc.h;
}

int inih_handler(
  void *configptr, const char *section, 
  const char *name, const char *value) {
  frameserver_config *fsc = (frameserver_config *)configptr;

  if (strncmp(section, "frameserver", 7) == 0) {
    if (strcmp(name, "v4ldev") == 0) {
      fsc->dev = std::string(value);
    }
    if (strcmp(name, "httpport") == 0) {
      fsc->httpport = atoi(value);
    }
    if (strcmp(name, "w") == 0) {
      fsc->w = atoi(value);
    }
    if (strcmp(name, "h") == 0) {
      fsc->h = atoi(value);
    }
    if (strcmp(name, "fps") == 0) {
      fsc->fps = atoi(value);
    }
    if (strcmp(name, "font") == 0) {
      fsc->fontfilename = std::string(value);
    }
  }

  if (strncmp(section, "stream_", 7) == 0) {
    stream *s = (stream *)fsc->streams_by_name[section];

    if (s == NULL) {
      fsc->streams_by_name[section] = new stream();
      s = (stream *)fsc->streams_by_name[section];
      s->set_id(fsc->get_last_id());
    }

    if (strcmp(name, "uri") == 0) {
      printf("value: %s", value);
      s->set_uri(std::string(value));
    }
    if (strcmp(name, "textoverlay") == 0) {
      s->set_textoverlay(std::string(value));
    }
    if (strcmp(name, "w") == 0) {
      s->set_w(atoi(value));
    }
    if (strcmp(name, "h") == 0) {
      s->set_h(atoi(value));
    }
    if (strcmp(name, "x") == 0) {
      s->set_x(atoi(value));
    }
    if (strcmp(name, "y") == 0) {
      s->set_y(atoi(value));
    }
    if (strcmp(name, "zindex") == 0) {
      s->set_zindex(atoi(value));
    }
  }
  return 0;
}

frameserver_config::frameserver_config() { last_id = 1; }

void frameserver_config::read_ini_config(std::string filename) {
  if (ini_parse(filename.c_str(), inih_handler, this) < 0) {
    std::cout << "Can't load " << filename << std::endl;
  }

  std::map<std::string, stream *>::iterator it = streams_by_name.begin();

  while (it != streams_by_name.end()) {
    streams.push_back(it->second);
    ++it;
  }

  // Sort the streams by zindex
  std::sort(streams.begin(), streams.end(), [](stream *s1, stream *s2) {
    return (s1->get_zindex() < s2->get_zindex());
  });

  for (int i = 0; i < (int)streams.size(); i++) {
    if (fontfilename.length()>0) {
      streams[i]->set_fontfilename(fontfilename);
    }
  }

  std::cout << validate() << std::endl;
}

}

