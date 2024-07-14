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

// void Analytic::draw_on_frame(NvDsBatchMeta *batch_meta) {
//   // TODO: drawing based on configuration for each source
//   NvDsFrameMeta *frame_meta = nvds_get_nth_frame_meta (batch_meta->frame_meta_list, 0);

//   NvDsDisplayMeta *display_meta = nvds_acquire_display_meta_from_pool(batch_meta);

//   // drawing configuration
//   NvOSD_LineParams *line_params = &display_meta->line_params[0];
//   // coordinate is coordinate after frame is resized from streammux
//   line_params->x1 = 100;
//   line_params->y1 = 440;
//   line_params->x2 = 1800;
//   line_params->y2 = 440;
//   line_params->line_width = 5;
//   // default values are 0, which make it invisible due to alpha = 0
//   line_params->line_color = (NvOSD_ColorParams){1.0, 0.0, 0.0, 1.0};
//   display_meta->num_lines++;

//   // drawing result of analytics
//   NvOSD_TextParams *text_params = &display_meta->text_params[0];
//   display_meta->num_labels++;
//   text_params->display_text = static_cast<char*>(g_malloc0(MAX_DISPLAY_LEN));
//   std::snprintf(text_params->display_text, MAX_DISPLAY_LEN, "In: %lu, Out: %lu",
//                 line_crossing_infos[FIRST_SOURCE][0].crossing_direction_counts[LineCrossDirection::RightToLeft],
//                 line_crossing_infos[FIRST_SOURCE][0].crossing_direction_counts[LineCrossDirection::LeftToRight]);

//   text_params->x_offset = 100;
//   text_params->y_offset = 440;

//   text_params->font_params.font_name = "Serif";
//   text_params->font_params.font_size = 12;
//   text_params->font_params.font_color.red = 1.0;
//   text_params->font_params.font_color.green = 1.0;
//   text_params->font_params.font_color.blue = 1.0;
//   text_params->font_params.font_color.alpha = 1.0;

//   text_params->set_bg_clr = TRUE;
//   text_params->text_bg_clr.red = 0.0;
//   text_params->text_bg_clr.green = 0.0;
//   text_params->text_bg_clr.blue = 0.0;
//   text_params->text_bg_clr.alpha = 1.0;

//   nvds_add_display_meta_to_frame(frame_meta, display_meta);
// }

void Analytic::update_parking_state(NvDsBatchMeta *batch_meta) {
  NvDsFrameMeta *frame_meta = nvds_get_nth_frame_meta(batch_meta->frame_meta_list, 0);
  NvDsMetaList *l_obj = nullptr;

  for (l_obj = frame_meta->obj_meta_list; l_obj != NULL;
       l_obj = l_obj->next) {
    NvDsObjectMeta *obj_meta = (NvDsObjectMeta *)(l_obj->data);
    // each item here must be car since I already fixed it in PGIE config

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
        }
      }
    }

  // remove stale objects
  // auto stale_object = std::remove_if(parking_states.begin(), parking_states.end(), [frame_meta](const auto &parking_state) {
  //   return frame_meta->ntp_timestamp - parking_state.second.last_seen > STALE_OBJECT_THRESHOLD;
  // });

  // parking_states.erase(stale_object, parking_states.end());
  }
}