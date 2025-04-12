#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#include "luckfox_mpi.h"
#include "yolov5.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video.hpp>  // 必须包含
#include <opencv2/opencv.hpp> // 或直接包含全部

#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <iostream>
#include <sys/mman.h>

#define DISP_WIDTH 480
#define DISP_HEIGHT 480

// Display size
int width = DISP_WIDTH;
int height = DISP_HEIGHT;

// Model size
int model_width = 640;
int model_height = 640;
float scale;
int leftPadding;
int topPadding;

cv::Mat letterbox(cv::Mat input)
{
    float scaleX = (float)model_width / (float)width;
    float scaleY = (float)model_height / (float)height;
    scale = scaleX < scaleY ? scaleX : scaleY;
    // printf("scale = %f\n", scale);

    int inputWidth = (int)((float)width * scale);
    int inputHeight = (int)((float)height * scale);

    leftPadding = (model_width - inputWidth) / 2;
    topPadding = (model_height - inputHeight) / 2;

    cv::Mat inputScale;
    cv::resize(input, inputScale, cv::Size(inputWidth, inputHeight), 0, 0, cv::INTER_LINEAR);
    cv::Mat letterboxImage(640, 640, CV_8UC3, cv::Scalar(114, 114, 114));
    cv::Rect roi(leftPadding, topPadding, inputWidth, inputHeight);
    inputScale.copyTo(letterboxImage(roi));

    return letterboxImage;
}

void mapCoordinates(int *x, int *y)
{
    int mx = *x - leftPadding;
    int my = *y - topPadding;
    *x = (int)((float)mx / scale);
    *y = (int)((float)my / scale);
}

int DRM_setup(int *fd, void **fb_map, uint32_t *fb_id, uint32_t *fb_size, drmModeCrtc **out_crtc, drmModeConnector **out_connector, drmModeModeInfo **out_mode)
{
    uint32_t local_fb_id = 0;
    *fd = open("/dev/dri/card0", O_RDWR);
    if (*fd < 0)
    {
        fprintf(stderr, "Failed to open DRM device: %s\n", strerror(errno));
        return -1;
    }

    drmModeRes *resources = drmModeGetResources(*fd);
    if (!resources)
    {
        fprintf(stderr, "Failed to get DRM resources\n");
        close(*fd);
        return -1;
    }

    drmModeConnector *connector = NULL;
    for (int i = 0; i < resources->count_connectors; i++)
    {
        connector = drmModeGetConnector(*fd, resources->connectors[i]);
        if (connector && connector->connection == DRM_MODE_CONNECTED)
        {
            break;
        }
        drmModeFreeConnector(connector);
    }
    if (!connector)
    {
        fprintf(stderr, "No connected DRM connector found\n");
        drmModeFreeResources(resources);
        close(*fd);
        return -1;
    }

    drmModeModeInfo *mode = NULL;
    for (int i = 0; i < connector->count_modes; i++)
    {
        if (connector->modes[i].hdisplay == DISP_WIDTH && connector->modes[i].vdisplay == DISP_HEIGHT)
        {
            mode = &connector->modes[i];
            break;
        }
    }
    if (!mode)
    {
        fprintf(stderr, "No matching display mode found\n");
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(*fd);
        return -1;
    }

    struct drm_mode_create_dumb create_dumb = {0};
    create_dumb.width = DISP_WIDTH;
    create_dumb.height = DISP_HEIGHT;
    create_dumb.bpp = 32;

    if (drmIoctl(*fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) < 0)
    {
        fprintf(stderr, "Failed to create dumb buffer: %s\n", strerror(errno));
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(*fd);
        return -1;
    }

    struct drm_mode_map_dumb map_dumb = {0};
    map_dumb.handle = create_dumb.handle;

    if (drmIoctl(*fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0)
    {
        fprintf(stderr, "Failed to map dumb buffer: %s\n", strerror(errno));
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(*fd);
        return -1;
    }

    *fb_map = mmap(0, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, map_dumb.offset);
    if (*fb_map == MAP_FAILED)
    {
        fprintf(stderr, "Failed to mmap framebuffer: %s\n", strerror(errno));
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(*fd);
        return -1;
    }

    if (drmModeAddFB(*fd, DISP_WIDTH, DISP_HEIGHT, 24, 32, create_dumb.pitch, create_dumb.handle, &local_fb_id) < 0)
    {
        fprintf(stderr, "Failed to add framebuffer: %s\n", strerror(errno));
        munmap(*fb_map, create_dumb.size);
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(*fd);
        return -1;
    }

    drmModeCrtc *crtc = drmModeGetCrtc(*fd, resources->crtcs[0]);
    if (!crtc)
    {
        fprintf(stderr, "Failed to get CRTC: %s\n", strerror(errno));
        munmap(*fb_map, create_dumb.size);
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(*fd);
        return -1;
    }

    if (drmModeSetCrtc(*fd, crtc->crtc_id, local_fb_id, 0, 0, &connector->connector_id, 1, mode) < 0)
    {
        fprintf(stderr, "Failed to set CRTC: %s\n", strerror(errno));
        drmModeFreeCrtc(crtc);
        munmap(*fb_map, create_dumb.size);
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(*fd);
        return -1;
    }

    *fb_id = local_fb_id;
    *fb_size = create_dumb.size;
    *out_crtc = crtc;
    *out_connector = connector;
    *out_mode = mode;

    drmModeFreeResources(resources);
    return 0;
}

static uint64_t get_time_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

int main(int argc, char *argv[])
{
    // Initialize YOLOv5 model
    rknn_app_context_t rknn_app_ctx;
    object_detect_result_list od_results;
    memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));
    init_yolov5_model("./model/best.rknn", &rknn_app_ctx);
    init_post_process();

    int fd;
    void *fb_map;
    uint32_t fb_id, fb_size;
    drmModeCrtc *crtc;
    drmModeConnector *connector;
    drmModeModeInfo *mode;
    if (DRM_setup(&fd, &fb_map, &fb_id, &fb_size, &crtc, &connector, &mode) != 0)
    {
        fprintf(stderr, "DRM setup failed\n");
        return -1;
    }

    // std::string input = argv[1];

    cv::VideoCapture cap;

    // cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    // cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap.open(0);

    // 检查摄像头是否成功打开
    // if (!cap.isOpened())
    // {
    //     std::cerr << "Error: Could not open camera" << std::endl;
    //     return -1;
    // }

    if (!cap.isOpened())
    {
        printf("Failed to open camera\n");
        return -1;
    }
    cv::Mat frame;

    while (true)
    {

        uint64_t start_time = get_time_ms();
        cap >> frame;

        cv::Mat letterboxImage = letterbox(frame);
        memcpy(rknn_app_ctx.input_mems[0]->virt_addr, letterboxImage.data, model_width * model_height * 3);
        inference_yolov5_model(&rknn_app_ctx, &od_results);

        cv::cvtColor(frame, frame, cv::COLOR_BGR2BGRA);
        cv::resize(frame, frame, cv::Size(DISP_WIDTH, DISP_HEIGHT));

        // 画框和概率
        char text[256];
        for (int i = 0; i < od_results.count; i++)
        {
            object_detect_result *det_result = &(od_results.results[i]);
            printf("%s @ (%d %d %d %d) %.3f\n", coco_cls_to_name(det_result->cls_id),
                   det_result->box.left, det_result->box.top,
                   det_result->box.right, det_result->box.bottom,
                   det_result->prop);
            int sX = (int)(det_result->box.left);
            int sY = (int)(det_result->box.top);
            int eX = (int)(det_result->box.right);
            int eY = (int)(det_result->box.bottom);
            mapCoordinates(&sX, &sY);
            mapCoordinates(&eX, &eY);
            cv::rectangle(frame, cv::Point(sX, sY), cv::Point(eX, eY), cv::Scalar(0, 255, 0), 3);
            sprintf(text, "%s %.1f%%", coco_cls_to_name(det_result->cls_id), det_result->prop * 100);
            cv::putText(frame, text, cv::Point(sX, sY - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
        }

        // 9. 复制 RGBA 数据到 framebuffer
        memcpy(fb_map, frame.data, fb_size);

        // 缩放或裁剪以匹配 SCREEN_WIDTH x SCREEN_HEIGHT
        // for (int y = 0; y < SCREEN_HEIGHT; y++) {
        //     for (int x = 0; x < SCREEN_WIDTH; x++) {
        //         int img_x = (x * img_width) / SCREEN_WIDTH;
        //         int img_y = (y * img_height) / SCREEN_HEIGHT;
        //         int pixel_idx = (img_y * img_width + img_x) * 4;

        //         uint8_t r = img_data[pixel_idx];
        //         uint8_t g = img_data[pixel_idx + 1];
        //         uint8_t b = img_data[pixel_idx + 2];
        //         uint8_t a = img_data[pixel_idx + 3];

        //         ((uint32_t*)fb_map)[y * SCREEN_WIDTH + x] = (a << 24) | (r << 16) | (g << 8) | b;
        //     }
        // }

        // drmModeCrtc *crtc = drmModeGetCrtc(fd, connector->encoder_id);
        drmModeSetCrtc(fd, crtc->crtc_id, fb_id, 0, 0, &connector->connector_id, 1, mode);

        // }
        uint64_t end_time = get_time_ms();

        double total_time = (end_time - start_time) / 1000.0; // 转换为秒
        printf("cost %.2f seconds\n", total_time);
        printf("FPS: %.1f\n", 1000.0 / (get_time_ms() - start_time));
        // if (cv::waitKey(0) == 'q')
        // break; // 按q或ESC退出
    }
    // Cleanup
    release_yolov5_model(&rknn_app_ctx);
    deinit_post_process();

    drmModeRmFB(fd, fb_id);
    munmap(fb_map, fb_size);
    // drmModeFreeCrtc(crtc);
    // drmModeFreeConnector(connector);
    // drmModeFreeResources(resources);
    close(fd);
    return 0;
}
