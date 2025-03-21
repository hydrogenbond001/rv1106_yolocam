cmake_minimum_required(VERSION 3.11)
project(whilt_show C)

# FILE(GLOB_RECURSE mpp_include mpp/include/rockchip)

include_directories(
    include
    include/libdrm
    src
    rga/include
    # ${mpp_include}
)




link_directories(${CMAKE_SOURCE_DIR}/lib)

# 添加源文件
set(SOURCES
    src/display_white.c
    # test.cpp
)

# 添加可执行文件
add_executable(whilt_show ${SOURCES})

find_library(RGA_LIBRARY NAMES rga PATHS ${CMAKE_SOURCE_DIR}/rga/lib)

target_link_libraries(whilt_show
    
    ${MPP_LIBRARY}
    drm
)

# link_directories(rga/lib)
# 链接必要的库
# target_link_libraries(whilt_show
#     drm
#     # pthread
#     rga
# )