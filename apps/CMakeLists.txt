add_executable(plm
  Analytic.cpp
  BufferLedger.cpp
  FrameBuffer.cpp
  Geometry.cpp
  main.cpp
  MovementAnalyzer.cpp
  Pipeline.cpp
  PipelineCallback.cpp
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)

find_package(CUDAToolkit REQUIRED)

find_package(OpenCV REQUIRED)

find_package(NVDS REQUIRED)

target_compile_features(plm PUBLIC cxx_std_20)

target_link_libraries(plm
  PRIVATE
    CUDA::cudart
    PkgConfig::GSTREAMER
    ${OpenCV_LIBS}
    ${NVDS_LIBRARIES}
)

target_include_directories(plm
  PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${NVDS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
