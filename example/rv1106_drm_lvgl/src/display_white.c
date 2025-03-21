#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/mman.h>
#include <time.h>  // 用于获取时间

#define DRM_DEVICE "/dev/dri/card0"
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 480
#define NUM_FRAMES 200  // 测试的帧数

// 获取当前时间（毫秒）
static uint64_t get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

int main() {
    int fd;
    drmModeConnector *connector = NULL;
    drmModeRes *resources = NULL;
    uint32_t fb_id = 0;

    // 打开 DRM 设备
    fd = open(DRM_DEVICE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open DRM device: %s\n", strerror(errno));
        return -1;
    }

    // 获取 DRM 资源
    resources = drmModeGetResources(fd);
    if (!resources) {
        fprintf(stderr, "Failed to get DRM resources: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    // 查找连接的显示器
    for (int i = 0; i < resources->count_connectors; i++) {
        connector = drmModeGetConnector(fd, resources->connectors[i]);
        if (connector && connector->connection == DRM_MODE_CONNECTED) {
            break;
        }
        drmModeFreeConnector(connector);
        connector = NULL;
    }

    if (!connector) {
        fprintf(stderr, "No connected DRM connector found.\n");
        drmModeFreeResources(resources);
        close(fd);
        return -1;
    }

    // 查找合适的显示模式
    drmModeModeInfo *mode = NULL;
    for (int i = 0; i < connector->count_modes; i++) {
        if (connector->modes[i].hdisplay == SCREEN_WIDTH &&
            connector->modes[i].vdisplay == SCREEN_HEIGHT) {
            mode = &connector->modes[i];
            break;
        }
    }

    if (!mode) {
        fprintf(stderr, "No matching display mode found.\n");
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(fd);
        return -1;
    }

    // 创建 Dumb Buffer
    struct drm_mode_create_dumb create_dumb = {0};
    create_dumb.width = SCREEN_WIDTH;
    create_dumb.height = SCREEN_HEIGHT;
    create_dumb.bpp = 32; // 32 bits per pixel (ARGB8888)

    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) < 0) {
        fprintf(stderr, "Failed to create dumb buffer: %s\n", strerror(errno));
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(fd);
        return -1;
    }

    // 映射 Dumb Buffer
    struct drm_mode_map_dumb map_dumb = {0};
    map_dumb.handle = create_dumb.handle;

    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0) {
        fprintf(stderr, "Failed to map dumb buffer: %s\n", strerror(errno));
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(fd);
        return -1;
    }

    void *fb_map = mmap(0, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_dumb.offset);
    if (fb_map == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap framebuffer: %s\n", strerror(errno));
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(fd);
        return -1;
    }

    // 创建 DRM Framebuffer
    if (drmModeAddFB(fd, SCREEN_WIDTH, SCREEN_HEIGHT, 24, 32, create_dumb.pitch, create_dumb.handle, &fb_id) < 0) {
        fprintf(stderr, "Failed to add framebuffer: %s\n", strerror(errno));
        munmap(fb_map, create_dumb.size);
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(fd);
        return -1;
    }

    // 设置 CRTC（显示控制器）
    drmModeCrtc *crtc = drmModeGetCrtc(fd, resources->crtcs[0]);
    if (!crtc) {
        fprintf(stderr, "Failed to get CRTC: %s\n", strerror(errno));
        munmap(fb_map, create_dumb.size);
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(fd);
        return -1;
    }

    if (drmModeSetCrtc(fd, crtc->crtc_id, fb_id, 0, 0, &connector->connector_id, 1, mode) < 0) {
        fprintf(stderr, "Failed to set CRTC: %s\n", strerror(errno));
        drmModeFreeCrtc(crtc);
        munmap(fb_map, create_dumb.size);
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        close(fd);
        return -1;
    }

    // 帧率测试
    uint64_t start_time = get_time_ms();
    for (int frame = 0; frame < NUM_FRAMES; frame++) {
        // 填充渐变颜色（从黑色到白色）
        uint32_t *pixels = (uint32_t *)fb_map;
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                uint8_t intensity = (x + y + frame) * 255 / (SCREEN_WIDTH + SCREEN_HEIGHT + NUM_FRAMES);
                pixels[y * SCREEN_WIDTH + x] = (0xFF << 24) | (intensity << 16) | (intensity << 8) | intensity;
            }
        }

        // 刷新显示
        drmModeSetCrtc(fd, crtc->crtc_id, fb_id, 0, 0, &connector->connector_id, 1, mode);
    }
    uint64_t end_time = get_time_ms();

    // 计算帧率
    double total_time = (end_time - start_time) / 1000.0;  // 转换为秒
    double fps = NUM_FRAMES / total_time;
    printf("Rendered %d frames in %.2f seconds (%.2f FPS)\n", NUM_FRAMES, total_time, fps);

    // 等待用户输入后退出
    getchar();

    // 清理资源
    drmModeRmFB(fd, fb_id);
    munmap(fb_map, create_dumb.size);
    drmModeFreeCrtc(crtc);
    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    close(fd);

    return 0;
}