#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <drm/drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#define DISP_WIDTH 480
#define DISP_HEIGHT 480
int drm_width = 480;
int drm_height = 480;

int drm_fd;
uint32_t drm_fb_id;
void *drm_fb_map;
size_t drm_fb_size;
drmModeCrtc *drm_crtc = NULL;
drmModeConnector *drm_connector = NULL;

int drm_setup()
{
    // 打开DRM设备
    drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (drm_fd < 0)
    {
        perror("Failed to open DRM device");
        return -1;
    }

    // 获取资源
    drmModeRes *resources = drmModeGetResources(drm_fd);
    if (!resources)
    {
        perror("Failed to get DRM resources");
        close(drm_fd);
        return -1;
    }

    // 查找连接的connector
    drm_connector = NULL;
    for (int i = 0; i < resources->count_connectors; i++)
    {
        drm_connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
        if (drm_connector && drm_connector->connection == DRM_MODE_CONNECTED && drm_connector->count_modes > 0)
        {
            break;
        }
        drmModeFreeConnector(drm_connector);
        drm_connector = NULL;
    }

    if (!drm_connector)
    {
        fprintf(stderr, "No connected DRM connector found\n");
        drmModeFreeResources(resources);
        close(drm_fd);
        return -1;
    }

    // 查找匹配的模式
    drmModeModeInfo *mode = NULL;
    for (int i = 0; i < drm_connector->count_modes; i++)
    {
        if (drm_connector->modes[i].hdisplay == DISP_WIDTH &&
            drm_connector->modes[i].vdisplay == DISP_HEIGHT)
        {
            mode = &drm_connector->modes[i];
            break;
        }
    }

    if (!mode)
    {
        fprintf(stderr, "No matching mode found\n");
        drmModeFreeConnector(drm_connector);
        drmModeFreeResources(resources);
        close(drm_fd);
        return -1;
    }

    // 创建 dumb buffer
    struct drm_mode_create_dumb create_dumb = {0};
    create_dumb.width = DISP_WIDTH;
    create_dumb.height = DISP_HEIGHT;
    create_dumb.bpp = 32;

    if (drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) < 0)
    {
        perror("Failed to create dumb buffer");
        drmModeFreeConnector(drm_connector);
        drmModeFreeResources(resources);
        close(drm_fd);
        return -1;
    }

    // 映射 dumb buffer
    struct drm_mode_map_dumb map_dumb = {0};
    map_dumb.handle = create_dumb.handle;

    if (drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0)
    {
        perror("Failed to map dumb buffer");
        drmModeFreeConnector(drm_connector);
        drmModeFreeResources(resources);
        close(drm_fd);
        return -1;
    }

    drm_fb_map = mmap(0, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, map_dumb.offset);
    if (drm_fb_map == MAP_FAILED)
    {
        perror("Failed to mmap framebuffer");
        drmModeFreeConnector(drm_connector);
        drmModeFreeResources(resources);
        close(drm_fd);
        return -1;
    }

    memset(drm_fb_map, 0, create_dumb.size); // 清屏
    drm_fb_size = create_dumb.size;

    // 创建 framebuffer
    if (drmModeAddFB(drm_fd, DISP_WIDTH, DISP_HEIGHT, 24, 32, create_dumb.pitch, create_dumb.handle, &drm_fb_id) < 0)
    {
        perror("Failed to add framebuffer");
        munmap(drm_fb_map, drm_fb_size);
        drmModeFreeConnector(drm_connector);
        drmModeFreeResources(resources);
        close(drm_fd);
        return -1;
    }

    // 获取 CRTC
    drm_crtc = drmModeGetCrtc(drm_fd, resources->crtcs[0]);
    if (!drm_crtc)
    {
        perror("Failed to get CRTC");
        munmap(drm_fb_map, drm_fb_size);
        drmModeFreeConnector(drm_connector);
        drmModeFreeResources(resources);
        close(drm_fd);
        return -1;
    }

    // 设置 CRTC
    if (drmModeSetCrtc(drm_fd, drm_crtc->crtc_id, drm_fb_id, 0, 0, &drm_connector->connector_id, 1, mode) < 0)
    {
        perror("Failed to set CRTC");
        drmModeFreeCrtc(drm_crtc);
        munmap(drm_fb_map, drm_fb_size);
        drmModeFreeConnector(drm_connector);
        drmModeFreeResources(resources);
        close(drm_fd);
        return -1;
    }

    drmModeFreeResources(resources);
    return 0;
}

void drm_cleanup()
{
    if (drm_crtc)
        drmModeFreeCrtc(drm_crtc);
    if (drm_connector)
        drmModeFreeConnector(drm_connector);
    if (drm_fb_map)
        munmap(drm_fb_map, drm_fb_size);
    if (drm_fd >= 0)
        close(drm_fd);
}
