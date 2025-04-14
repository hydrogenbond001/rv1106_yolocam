cmake_minimum_required(VERSION 3.6)

project(rknn_yolov5_demo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--allow-shlib-undefined")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,--allow-shlib-undefined")

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_yolov5_demo_${CMAKE_SYSTEM_NAME})

set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/../../) #example/luckfox_zero_copy/CMakeLists.txt
set(RKNN_RT_LIB ${RKNN_API_PATH}/3rdparty/rknpu2/Linux/armhf-uclibc/librknnmrt.so)

message("RKNN_API_PATH = ${RKNN_API_PATH}")
include_directories(${RKNN_API_PATH}/3rdparty/allocator/drm)

include_directories(${RKNN_API_PATH}/3rdparty/rknpu2/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

set(OpenCV_DIR "${RKNN_API_PATH}/lib/cmake/opencv4")
find_package(OpenCV REQUIRED)
# rknn_yolov5_demo
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(rknn_yolov5_demo
            src/main.cc
            src/postprocess.cc
)

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,${RKNN_API_PATH}/lib:/usr/lib")


target_link_libraries(rknn_yolov5_demo
${RKNN_RT_LIB}
${OpenCV_LIBS}
${RKNN_API_PATH}/lib
)

# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install/rknn_yolov5_demo_${CMAKE_SYSTEM_NAME})
install(TARGETS rknn_yolov5_demo DESTINATION ./)


install(PROGRAMS ${RKNN_RT_LIB} DESTINATION lib)
install(DIRECTORY model DESTINATION ./)
