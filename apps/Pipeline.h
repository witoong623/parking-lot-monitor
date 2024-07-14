#pragma once

#include <vector>

#include <gst/gst.h>
#include <glib.h>

#include "Analytic.h"
#include "FrameBuffer.h"

class Pipeline {
  public:
    Pipeline(GMainLoop *loop, gchar *config_filepath);

    ~Pipeline();

    guint bus_watch_id;
    GstElement *pipeline;

    inline Analytic &analytic() { return _analytic; }
  private:
    std::vector<GstElement *> create_sources(gchar *config_filepath);
    GstElement *create_source_bin (guint index, gchar *uri);
    // should be called after all elements are configured
    void register_probs();

    GMainLoop *loop;

    GstElement *streammux;
    GstElement *sink;
    GstElement *pgie;
    GstElement *tracker;
    GstElement *nvosd;
    GstElement *tiler;
    GstElement *nvdslogger;
    GstElement *nvdsanalytics;

    FrameBuffer _frame_buffer;
    Analytic _analytic;
    std::vector<GstElement *> sources;
};
