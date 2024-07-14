#include "PipelineCallback.h"

#include <string>

#include <glib.h>
#include <gst/gst.h>
#include "gst-nvmessage.h"
#include "gstnvdsmeta.h"
#include "nvbufsurface.h"
#include "nvds_analytics_meta.h"
#include "opencv2/opencv.hpp"

#include "FrameBuffer.h"
#include "Pipeline.h"


#define GST_CAPS_FEATURES_NVMM "memory:NVMM"

gboolean pipeline_bus_watch (GstBus * bus, GstMessage * msg, gpointer data) {
  GMainLoop *loop = (GMainLoop *) data;
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_WARNING:
    {
      gchar *debug;
      GError *error;
      gst_message_parse_warning (msg, &error, &debug);
      g_printerr ("WARNING from element %s: %s\n",
          GST_OBJECT_NAME (msg->src), error->message);
      g_free (debug);
      g_printerr ("Warning: %s\n", error->message);
      g_error_free (error);
      break;
    }
    case GST_MESSAGE_ERROR:
    {
      gchar *debug;
      GError *error;
      gst_message_parse_error (msg, &error, &debug);
      g_printerr ("ERROR from element %s: %s\n",
          GST_OBJECT_NAME (msg->src), error->message);
      if (debug)
        g_printerr ("Error details: %s\n", debug);
      g_free (debug);
      g_error_free (error);
      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_ELEMENT:
    {
      if (gst_nvmessage_is_stream_eos (msg)) {
        guint stream_id;
        if (gst_nvmessage_parse_stream_eos (msg, &stream_id)) {
          g_print ("Got EOS from stream %d\n", stream_id);
        }
      }
      break;
    }
    default:
      break;
  }
  return TRUE;
}

void src_newpad_cb (GstElement * decodebin, GstPad * decoder_src_pad, gpointer data) {
  GstCaps *caps = gst_pad_get_current_caps (decoder_src_pad);
  if (!caps) {
    caps = gst_pad_query_caps (decoder_src_pad, NULL);
  }
  const GstStructure *str = gst_caps_get_structure (caps, 0);
  const gchar *name = gst_structure_get_name (str);
  GstElement *source_bin = (GstElement *) data;
  GstCapsFeatures *features = gst_caps_get_features (caps, 0);

  /* Need to check if the pad created by the decodebin is for video and not
   * audio. */
  if (!strncmp (name, "video", 5)) {
    /* Link the decodebin pad only if decodebin has picked nvidia
     * decoder plugin nvdec_*. We do this by checking if the pad caps contain
     * NVMM memory features. */
    if (gst_caps_features_contains (features, GST_CAPS_FEATURES_NVMM)) {
      /* Get the source bin ghost pad */
      GstPad *bin_ghost_pad = gst_element_get_static_pad (source_bin, "src");
      if (!gst_ghost_pad_set_target (GST_GHOST_PAD (bin_ghost_pad),
              decoder_src_pad)) {
        g_printerr ("Failed to link decoder src pad to source bin ghost pad\n");
      }
      gst_object_unref (bin_ghost_pad);
    } else {
      g_printerr ("Error: Decodebin did not pick nvidia decoder plugin.\n");
    }
  }
}

GstPadProbeReturn analytics_callback_tiler_prob (GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
  Pipeline *pipeline = static_cast<Pipeline *>(user_data);
  GstBuffer *buf = static_cast<GstBuffer *>(info->data);
  NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta (buf);
  NvDsFrameMeta *frame_meta = nvds_get_nth_frame_meta (batch_meta->frame_meta_list, 0);
  NvDsMetaList * l_obj = nullptr;

  g_print("====== start frame %d ======\n", frame_meta->frame_num);

  for (l_obj = frame_meta->obj_meta_list; l_obj != NULL;
        l_obj = l_obj->next) {
    NvDsObjectMeta *obj_meta = (NvDsObjectMeta *) (l_obj->data);
    // each item here must be car since I already fixed it in PGIE config

    // Access attached user meta for each object
    for (NvDsMetaList *l_user_meta = obj_meta->obj_user_meta_list; l_user_meta != NULL;
            l_user_meta = l_user_meta->next) {
        NvDsUserMeta *user_meta = (NvDsUserMeta *) (l_user_meta->data);
        if(user_meta->base_meta.meta_type == NVDS_USER_OBJ_META_NVDSANALYTICS)
        {
            NvDsAnalyticsObjInfo * user_meta_data = (NvDsAnalyticsObjInfo *)user_meta->user_meta_data;
            if (user_meta_data->roiStatus.size() == 0) {
                // object status  ROI:RF-1 in RF-1
                // g_print ("object status %s in %s\n", user_meta_data->objStatus.c_str(), user_meta_data->roiStatus[0].c_str());
                g_print ("object status %s outside ROI\n", user_meta_data->objStatus.c_str());
            }
        }
    }
}

  // for (NvDsMetaList * l_user = frame_meta->frame_user_meta_list;
  //       l_user != NULL; l_user = l_user->next) {
  //   NvDsUserMeta *user_meta = (NvDsUserMeta *) l_user->data;
  //   if (user_meta->base_meta.meta_type != NVDS_USER_FRAME_META_NVDSANALYTICS) {
  //     continue;
  //   }

  //   NvDsAnalyticsFrameMeta *meta = (NvDsAnalyticsFrameMeta *) user_meta->user_meta_data;
  //   for (auto &[k, v] : meta->objInROIcnt) {
  //     g_print("ROI: %s, count: %d\n", k.c_str(), v);
  //   }
  // }

  g_print("====== end frame %d ======\n", frame_meta->frame_num);

  return GST_PAD_PROBE_OK;
}
