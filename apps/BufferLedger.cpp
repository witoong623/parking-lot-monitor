#include "BufferLedger.h"


MemoryBuffer::MemoryBuffer(unsigned int width, unsigned int height,
                           MemoryType frame_type, int *ref_count):
                           ref_count(new int(0)) {
  switch (frame_type) {
    case MemoryType::RGB:
      memory = cv::Mat(height, width, CV_8UC3);
      break;
    case MemoryType::RGBA:
      memory = cv::Mat(height, width, CV_8UC4);
      break;
    case MemoryType::NV12:
      memory = cv::Mat(height + height / 2, width, CV_8UC1);
      break;
  }
}

MemoryBuffer::~MemoryBuffer() {
  if (ref_count != nullptr) {
    if (*ref_count > 0) {
      (*ref_count)--;
    }
  }
}

MemoryBuffer::MemoryBuffer(const MemoryBuffer& other) {
  memory = other.memory;
  ref_count = other.ref_count;
  if (ref_count != nullptr) {
    (*ref_count)++;
  }
}
