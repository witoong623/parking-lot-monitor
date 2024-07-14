#pragma once

#include <chrono>
#include <unordered_map>

#include <glib.h>
#include "gstnvdsmeta.h"

#include "FrameBuffer.h"
#include "MovementAnalyzer.h"


struct ParkingState {
  unsigned long object_id;
  std::chrono::time_point<std::chrono::system_clock> first_seen;
  std::chrono::time_point<std::chrono::system_clock> last_seen;
};


class Analytic {
  public:
    Analytic();

    void update_parking_state(NvDsBatchMeta *batch_meta);
  private:
    // object id -> parking state
    std::unordered_map<unsigned long, ParkingState> parking_states;
};
