cmake_minimum_required(VERSION 3.10)
project(lvgl)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-fPIC -Wall -Wno-error -O2 -g)

include_directories(
    include
    include/libdrm
    lv_drm
    lv_drm/rlottie/inc
    generated
    custom
    generated/guider_customer_fonts
    generated/guider_fonts
    generated/images
    src
    mpp/include/rockchip
)

link_directories(${CMAKE_SOURCE_DIR}/lib)

option(ENABLE_LOTTIE_SUPPORT "Enable Lottie support" OFF)

if(ENABLE_LOTTIE_SUPPORT)
        add_subdirectory(${CMAKE_SOURCE_DIR}/lv_drm/rlottie)
endif()

add_subdirectory(lv_drm/lvgl)
add_subdirectory(lv_drm/lv_drivers)

add_definitions(-DUSE_DRM -DUSE_EVDEV)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_DRM -DUSE_EVDEV")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_DRM -DUSE_EVDEV")

target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} rlottie/inc/)

FILE(GLOB_RECURSE app_sources generated/*.c custom/*.c src/main.c)

add_executable(my_lvglApp ${app_sources})

target_compile_definitions(my_lvglApp PRIVATE LV_LVGL_H_INCLUDE_SIMPLE)

target_link_libraries(my_lvglApp
    lvgl
    lv_drivers
    lvgl::drivers
    # lvgl::examples
    # lvgl::demos
    drm
    m
    pthread
)

if(ENABLE_LOTTIE_SUPPORT)
    target_link_libraries(my_lvglApp rlottie::rlottie)
endif()

# 定义显示全白图像的源文件
set(WHITE_SHOW_SOURCES
    src/display_white.c
)

# find_library(MPP_LIB rockchip_mpp)

# 添加显示全白图像的可执行文件
add_executable(whilt_show ${WHITE_SHOW_SOURCES})
target_link_libraries(whilt_show
    drm
    pthread
    # mpp/lib
)