#pragma once

#include <vector>
#include <unordered_map>

#include "opencv2/opencv.hpp"


enum class MemoryType {
  RGB,
  RGBA,
  NV12,
};


class MemoryBuffer {
  public:
    explicit MemoryBuffer(unsigned int width, unsigned int height, MemoryType frame_type, int *ref_count);
    ~MemoryBuffer();

    // copy constructor
    MemoryBuffer(const MemoryBuffer& other);

    MemoryBuffer(MemoryBuffer&&) = delete;
    MemoryBuffer& operator=(const MemoryBuffer& other) = delete;
    MemoryBuffer& operator=(MemoryBuffer&&) = delete;

    inline cv::Mat get_memory() { return memory; }
  private:
    cv::Mat memory;
    int *ref_count = nullptr;
};
