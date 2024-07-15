#include "Analytic.h"

#include <algorithm>
#include <cstdio>
#include <vector>

#include "gstnvdsmeta.h"
#include "nvds_analytics_meta.h"
#include <opencv2/opencv.hpp>

#include "FrameBuffer.h"
#include "Geometry.h"
#include "MovementAnalyzer.h"

constexpr gint STALE_OBJECT_THRESHOLD = 250;
constexpr guint FIRST_SOURCE = 0;
constexpr guint MAX_DISPLAY_LEN = 64;

Analytic::Analytic() {}

void Analytic::update_parking_state(NvDsBatchMeta *batch_meta) {
  NvDsFrameMeta *frame_meta = nvds_get_nth_frame_meta(batch_meta->frame_meta_list, 0);
  NvDsMetaList *l_obj = nullptr;

  for (l_obj = frame_meta->obj_meta_list; l_obj != NULL;
       l_obj = l_obj->next) {
    NvDsObjectMeta *obj_meta = (NvDsObjectMeta *)(l_obj->data);
    // each item here must be car since I already fixed it in PGIE config

    bool in_roi = false;
    // Access attached user meta for each object
    for (NvDsMetaList *l_user_meta = obj_meta->obj_user_meta_list; l_user_meta != NULL;
         l_user_meta = l_user_meta->next) {
      NvDsUserMeta *user_meta = (NvDsUserMeta *)(l_user_meta->data);
      if (user_meta->base_meta.meta_type == NVDS_USER_OBJ_META_NVDSANALYTICS) {
        NvDsAnalyticsObjInfo *user_meta_data = (NvDsAnalyticsObjInfo *)user_meta->user_meta_data;
        if (user_meta_data->roiStatus.size() > 0) {
          // object is already in one of the ROIs
          if (parking_states.find(obj_meta->object_id) == parking_states.end()) {
            // object is not in the parking_states
            parking_states[obj_meta->object_id] = {obj_meta->object_id, std::chrono::system_clock::now(), std::chrono::system_clock::now()};
          } else {
            // object is already in the parking_states
            parking_states[obj_meta->object_id].last_seen = std::chrono::system_clock::now();
          }

          in_roi = true;
          parking_states[obj_meta->object_id].last_seen_frame_num = frame_meta->frame_num;
        }
      }
    }

    if (!in_roi) {
      continue;
    }

    auto parking_state = parking_states[obj_meta->object_id];
    auto parking_duration = std::chrono::duration_cast<std::chrono::seconds>(parking_state.last_seen - parking_state.first_seen).count();

    auto min = parking_duration / 60;
    auto sec = parking_duration % 60;

    NvDsDisplayMeta *display_meta = nvds_acquire_display_meta_from_pool(batch_meta);
    display_meta->num_labels++;

    NvOSD_TextParams *text_params = &display_meta->text_params[0];
    text_params->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
    std::snprintf(text_params->display_text, MAX_DISPLAY_LEN, "Park for %ld:%ld", min, sec);

    auto x = obj_meta->detector_bbox_info.org_bbox_coords.left;
    auto y = obj_meta->detector_bbox_info.org_bbox_coords.top;
    auto width = obj_meta->detector_bbox_info.org_bbox_coords.width;
    auto height = obj_meta->detector_bbox_info.org_bbox_coords.height;

    text_params->x_offset = x + (width / 2);
    text_params->y_offset = y - 14;

    text_params->font_params.font_name = "Serif";
    text_params->font_params.font_size = 12;
    text_params->font_params.font_color.red = 1.0;
    text_params->font_params.font_color.green = 1.0;
    text_params->font_params.font_color.blue = 1.0;
    text_params->font_params.font_color.alpha = 1.0;

    text_params->set_bg_clr = TRUE;
    text_params->text_bg_clr.red = 0.0;
    text_params->text_bg_clr.green = 0.0;
    text_params->text_bg_clr.blue = 0.0;
    text_params->text_bg_clr.alpha = 1.0;

    nvds_add_display_meta_to_frame(frame_meta, display_meta);
  }

  // remove stale objects
  auto it = parking_states.begin();
  while (it != parking_states.end()) {
    if (frame_meta->frame_num - it->second.last_seen_frame_num > STALE_OBJECT_THRESHOLD) {
      parking_states.erase(it++);
    } else {
      ++it;
    }
  }
}