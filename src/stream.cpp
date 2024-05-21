#include "stream.hpp"

#include <stdbool.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "rgbimage.hpp"
#include "textrenderer.hpp"

GstFlowReturn new_sample(GstElement *sink, stream *s) {
  GstSample *sample;

  g_signal_emit_by_name(sink, "pull-sample", &sample);
  if (sample) {
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstMapInfo map_info;

    if (!gst_buffer_map(buffer, &map_info, GST_MAP_READ)) {
      gst_buffer_unmap(buffer, &map_info);

      return GST_FLOW_ERROR;
    }

    rgbimage *text = s->getRenderedOverlaytext();
    rgbimage frame;
    frame.setBuffer(map_info.data);
    frame.set_w(s->get_w());
    frame.set_h(s->get_h());

    if (s->getRenderedOverlaytext() != NULL) {
      frame.paste(text, 0, frame.get_h() - text->get_h());
    }
    s->setFrame(frame.getBuffer(), s->get_w(), s->get_h());
    frame.setBuffer(NULL);

    gst_buffer_unmap((buffer), &map_info);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
  }

  return GST_FLOW_ERROR;
}

stream::stream(std::string uri) {
  this->uri = uri;
  name = "";
  textoverlay = "";
  videoflip = "";

  w = h = x = y = INT_MIN;

  keep_alive = 0;
  stop_signal = 0;

  gst_pipeline_str = "";
  gst_appsinkname = "tsappsink";
  gst_videoflip = "";

  pipeline = NULL;
  appsink = NULL;
  bus = NULL;

  frame = NULL;
}

stream::stream() {
  uri = "";
  name = "";
  textoverlay = "";
  videoflip = "";

  w = h = x = y = INT_MIN;

  keep_alive = 0;
  stop_signal = 0;

  gst_pipeline_str = "";
  gst_appsinkname = "tsappsink";
  gst_videoflip = "";

  pipeline = NULL;
  appsink = NULL;
  bus = NULL;

  frame = NULL;
}

stream::~stream() {
  if (streamthread.joinable()) {
    std::cout << "Streaming thread stopping..." << std::endl;
    streamthread.join();
  }

  delete frame;
  frame = NULL;
  delete overlaytext;
  overlaytext = NULL;
}

void stream::setFrame(uint8_t *buffer, int w, int h) {
  frame->frombuffer(buffer, w, h);
}

rgbimage *stream::getRenderedOverlaytext() {
  if (textoverlay.length() > 0) {
    return overlaytext;
  } else {
    return NULL;
  }
}

// Creates a string launcher command for a stream
void stream::mkpipeline_s() {
  if (uri.rfind("http", 0) == 0) {
    gst_pipeline_str = "souphttpsrc location=";
    gst_pipeline_str += uri;
    gst_pipeline_str += "%s do-timestamp=true ! multipartdemux ! jpegdec ";
    gst_pipeline_str += gst_videoflip;
    gst_pipeline_str += "! videoscale method=lanczos ";
    gst_pipeline_str += "! videoconvert ";
    gst_pipeline_str += "! video/x-raw,width=";
    gst_pipeline_str += std::to_string(w);
    gst_pipeline_str += ",height=";
    gst_pipeline_str += std::to_string(h);
    gst_pipeline_str += ",format=RGB ";
    gst_pipeline_str += "! appsink name=";
    gst_pipeline_str += gst_appsinkname;
    gst_pipeline_str += " drop=true max-buffers=25 emit-signals=True ";
  }
  if (uri.rfind("rtsp", 0) == 0) {
    gst_pipeline_str = "rtspsrc location=";
    gst_pipeline_str += uri;
    gst_pipeline_str += " latency=0 protocols=tcp ";
    gst_pipeline_str +=
        "! rtph264depay ! avdec_h264 skip-frame=1 output-corrupt=false ";
    gst_pipeline_str += gst_videoflip;
    gst_pipeline_str += "! videoscale method=lanczos ";
    gst_pipeline_str += "! videoconvert ";
    gst_pipeline_str += "! video/x-raw,width=";
    gst_pipeline_str += std::to_string(w);
    gst_pipeline_str += ",height=";
    gst_pipeline_str += std::to_string(h);
    gst_pipeline_str += ",format=RGB ";
    gst_pipeline_str += "! appsink name=";
    gst_pipeline_str += gst_appsinkname;
    gst_pipeline_str += " drop=true max-buffers=25 emit-signals=True ";
  }
  if (uri.rfind("/", 0) == 0) {
    gst_pipeline_str = "multifilesrc location=\"";
    gst_pipeline_str += uri;
    gst_pipeline_str += "\" loop=true ";
    gst_pipeline_str += "! decodebin ";
    gst_pipeline_str += gst_videoflip;
    gst_pipeline_str += "! videoscale method=lanczos ";
    gst_pipeline_str += "! videoconvert ";
    gst_pipeline_str += "! video/x-raw,width=";
    gst_pipeline_str += std::to_string(w);
    gst_pipeline_str += ",height=";
    gst_pipeline_str += std::to_string(h);
    gst_pipeline_str += ",format=RGB ";
    gst_pipeline_str += "! appsink name=";
    gst_pipeline_str += gst_appsinkname;
    gst_pipeline_str += " drop=true max-buffers=25 emit-signals=True ";
  }
}

void *stream_thread(void *sm) {
  stream *s = (stream *)sm;

  s->start();

  return NULL;
}

void stream::open() {
  delete frame;
  frame = new rgbimage(w, h);

  textrenderer tr = textrenderer();
  tr.setFontFilename(this->fontfilename);
  tr.setText(textoverlay);
  tr.setFontSize(20);

  overlaytext = tr.render_with_shadow();

  mkpipeline_s();

  streamthread = std::thread(stream_thread, (void *)this);
}

int process_gst_message(GstMessage *msg) {
  int temp = 1;
  if ((msg) != NULL) {
    switch (GST_MESSAGE_TYPE(msg)) {
      case GST_MESSAGE_STATE_CHANGED: {
        // GstState old_state, new_state;

        // gst_message_parse_state_changed (msg, &old_state, &new_state, NULL);
        // g_print ("Element %s changed state from %s to %s.\n",
        //     GST_OBJECT_NAME (msg->src),
        //     gst_element_state_get_name (old_state),
        //     gst_element_state_get_name (new_state)
        // );
        break;
      }
      case GST_MESSAGE_EOS: {
        std::cout << "GST_MESSAGE_EOS" << std::endl;
        temp = 0;
        break;
      }
      case GST_MESSAGE_ERROR: {
        std::cout << "GST_MESSAGE_ERROR" << std::endl;
        temp = 0;
        break;
      }
      default: {
        std::cout << "Other message received" << std::endl;
        break;
      }
    }
  }
  return temp;
}

void stream::start() {
  volatile int repeatloop = 1;
  while (repeatloop) {
    pipeline = gst_parse_launch(gst_pipeline_str.c_str(), NULL);

    if (!keep_alive) {
      repeatloop = 0;
    }

    if (pipeline != NULL) {
      appsink = gst_bin_get_by_name(GST_BIN(pipeline), gst_appsinkname.c_str());

      g_signal_connect(appsink, "new-sample", G_CALLBACK(new_sample), this);

      gst_element_set_state(pipeline, GST_STATE_PLAYING);

      // Keeps on processing messages
      volatile int processmessages = 1;
      while (processmessages && !stop_signal) {
        bus = gst_element_get_bus(pipeline);
        GstMessage *msg;

        msg = gst_bus_timed_pop_filtered(
            bus, GST_CLOCK_TIME_NONE,
            (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS |
                             GST_MESSAGE_STATE_CHANGED));

        processmessages = process_gst_message(msg);
        if (msg != NULL) gst_message_unref(msg);
        msg = NULL;
      }

      if (keep_alive) {
        std::cout << "Sleeping 1s..." << std::endl;
        g_usleep(1000000);
      }
    }
  }
}

void stream::close() {
  keep_alive = 0;
  stop_signal = 1;

  if GST_IS_ELEMENT (appsink) {
    gst_element_set_state(appsink, GST_STATE_PAUSED);
    gst_element_set_state(appsink, GST_STATE_READY);
    gst_element_set_state(appsink, GST_STATE_NULL);
    gst_object_unref(appsink);
    appsink = NULL;
  }

  if GST_IS_ELEMENT (pipeline) {
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
    gst_element_set_state(pipeline, GST_STATE_READY);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = NULL;
  }

  if (bus != NULL) gst_object_unref(bus);
  bus = NULL;

  if (streamthread.joinable()) {
    std::cout << "Streaming thread stopping..." << std::endl;
    streamthread.join();
  }
  std::cout << "Streaming thread stopped." << std::endl;
}
