#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
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

#include "rtsp_demo.h"
#include "luckfox_mpi.h"
#include "yolov5.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <iostream>
#include <sys/mman.h>
#define DISP_WIDTH 480
#define DISP_HEIGHT 480

// disp size
int width = DISP_WIDTH;
int height = DISP_HEIGHT;

// model size
int model_width = 640;
int model_height = 640;
float scale;
int leftPadding;
int topPadding;

// DRM全局变量
int drm_fd;
uint32_t drm_fb_id;
void *drm_fb_map;
uint32_t drm_fb_size;
drmModeCrtc *drm_crtc;
drmModeConnector *drm_connector;

cv::Mat letterbox(cv::Mat input)
{
	float scaleX = (float)model_width / (float)width;
	float scaleY = (float)model_height / (float)height;
	scale = scaleX < scaleY ? scaleX : scaleY;
	// printf("venc init success\n");
	int inputWidth = (int)((float)width * scale);
	int inputHeight = (int)((float)height * scale);

	leftPadding = (model_width - inputWidth) / 2;
	topPadding = (model_height - inputHeight) / 2;

	cv::Mat inputScale;
	cv::resize(input, inputScale, cv::Size(inputWidth, inputHeight), 0, 0, cv::INTER_LINEAR);
	cv::Mat letterboxImage(640, 640, CV_8UC3, cv::Scalar(0, 0, 0));
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

int drm_setup()
{
	// 打开DRM设备
	drm_fd = open("/dev/dri/card0", O_RDWR);
	if (drm_fd < 0)
	{
		printf("Failed to open DRM device\n");
		return -1;
	}

	// 获取资源
	drmModeRes *resources = drmModeGetResources(drm_fd);
	if (!resources)
	{
		printf("Failed to get DRM resources\n");
		close(drm_fd);
		return -1;
	}

	// 查找连接的connector
	drm_connector = NULL;
	for (int i = 0; i < resources->count_connectors; i++)
	{
		drm_connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
		if (drm_connector && drm_connector->connection == DRM_MODE_CONNECTED)
		{
			break;
		}
		drmModeFreeConnector(drm_connector);
		drm_connector = NULL;
	}

	if (!drm_connector)
	{
		printf("No connected DRM connector found\n");
		drmModeFreeResources(resources);
		close(drm_fd);
		return -1;
	}

	// 查找匹配的显示模式
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
		printf("No matching display mode found\n");
		drmModeFreeConnector(drm_connector);
		drmModeFreeResources(resources);
		close(drm_fd);
		return -1;
	}

	// 创建dumb buffer
	struct drm_mode_create_dumb create_dumb = {0};
	create_dumb.width = DISP_WIDTH;
	create_dumb.height = DISP_HEIGHT;
	create_dumb.bpp = 32;

	if (drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) < 0)
	{
		printf("Failed to create dumb buffer\n");
		drmModeFreeConnector(drm_connector);
		drmModeFreeResources(resources);
		close(drm_fd);
		return -1;
	}

	// 映射dumb buffer
	struct drm_mode_map_dumb map_dumb = {0};
	map_dumb.handle = create_dumb.handle;

	if (drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0)
	{
		printf("Failed to map dumb buffer\n");
		drmModeFreeConnector(drm_connector);
		drmModeFreeResources(resources);
		close(drm_fd);
		return -1;
	}

	drm_fb_map = mmap(0, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, map_dumb.offset);
	if (drm_fb_map == MAP_FAILED)
	{
		printf("Failed to mmap framebuffer\n");
		drmModeFreeConnector(drm_connector);
		drmModeFreeResources(resources);
		close(drm_fd);
		return -1;
	}

	// 创建framebuffer
	if (drmModeAddFB(drm_fd, DISP_WIDTH, DISP_HEIGHT, 24, 32, create_dumb.pitch, create_dumb.handle, &drm_fb_id) < 0)
	{
		printf("Failed to add framebuffer\n");
		munmap(drm_fb_map, create_dumb.size);
		drmModeFreeConnector(drm_connector);
		drmModeFreeResources(resources);
		close(drm_fd);
		return -1;
	}

	drm_fb_size = create_dumb.size;

	// 获取CRTC
	drm_crtc = drmModeGetCrtc(drm_fd, resources->crtcs[0]);
	if (!drm_crtc)
	{
		printf("Failed to get CRTC\n");
		munmap(drm_fb_map, drm_fb_size);
		drmModeFreeConnector(drm_connector);
		drmModeFreeResources(resources);
		close(drm_fd);
		return -1;
	}

	// 设置CRTC
	if (drmModeSetCrtc(drm_fd, drm_crtc->crtc_id, drm_fb_id, 0, 0, &drm_connector->connector_id, 1, mode) < 0)
	{
		printf("Failed to set CRTC\n");
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
	if (drm_fb_id)
	{
		drmModeRmFB(drm_fd, drm_fb_id);
	}
	if (drm_fb_map)
	{
		munmap(drm_fb_map, drm_fb_size);
	}
	if (drm_crtc)
	{
		drmModeFreeCrtc(drm_crtc);
	}
	if (drm_connector)
	{
		drmModeFreeConnector(drm_connector);
	}
	if (drm_fd >= 0)
	{
		close(drm_fd);
	}
}

static uint64_t get_time_ms()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}
int main(int argc, char *argv[])
{
	system("RkLunch-stop.sh");
	RK_S32 s32Ret = 0;
	int sX, sY, eX, eY;

	// 初始化DRM显示
	if (drm_setup() != 0)
	{
		printf("DRM setup failed\n");
		return -1;
	}

	// Rknn model
	char text[16];
	rknn_app_context_t rknn_app_ctx;
	object_detect_result_list od_results;
	int ret;
	const char *model_path = "./model/best.rknn";
	memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));
	init_yolov5_model(model_path, &rknn_app_ctx);
	printf("init rknn model success!\n");
	init_post_process();

	// h264_frame
	VENC_STREAM_S stFrame;
	stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));
	RK_U64 H264_PTS = 0;
	RK_U32 H264_TimeRef = 0;
	VIDEO_FRAME_INFO_S stViFrame;

	// Create Pool
	MB_POOL_CONFIG_S PoolCfg;
	memset(&PoolCfg, 0, sizeof(MB_POOL_CONFIG_S));
	PoolCfg.u64MBSize = width * height * 3;
	PoolCfg.u32MBCnt = 1;
	PoolCfg.enAllocType = MB_ALLOC_TYPE_DMA;
	// PoolCfg.bPreAlloc = RK_FALSE;
	MB_POOL src_Pool = RK_MPI_MB_CreatePool(&PoolCfg);
	printf("Create Pool success !\n");

	// Get MB from Pool
	MB_BLK src_Blk = RK_MPI_MB_GetMB(src_Pool, width * height * 3, RK_TRUE);

	// Build h264_frame
	VIDEO_FRAME_INFO_S h264_frame;
	h264_frame.stVFrame.u32Width = width;
	h264_frame.stVFrame.u32Height = height;
	h264_frame.stVFrame.u32VirWidth = width;
	h264_frame.stVFrame.u32VirHeight = height;
	h264_frame.stVFrame.enPixelFormat = RK_FMT_RGB888;
	h264_frame.stVFrame.u32FrameFlag = 160;
	h264_frame.stVFrame.pMbBlk = src_Blk;
	unsigned char *data = (unsigned char *)RK_MPI_MB_Handle2VirAddr(src_Blk);
	cv::Mat frame(cv::Size(width, height), CV_8UC3, data);

	// rkaiq init
	RK_BOOL multi_sensor = RK_FALSE;
	const char *iq_dir = "/etc/iqfiles";
	rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
	// hdr_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
	SAMPLE_COMM_ISP_Init(0, hdr_mode, multi_sensor, iq_dir);
	SAMPLE_COMM_ISP_Run(0);

	// rkmpi init
	if (RK_MPI_SYS_Init() != RK_SUCCESS)
	{
		RK_LOGE("rk mpi sys init fail!");
		return -1;
	}

	// rtsp init
	rtsp_demo_handle g_rtsplive = NULL;
	rtsp_session_handle g_rtsp_session;
	g_rtsplive = create_rtsp_demo(554);
	g_rtsp_session = rtsp_new_session(g_rtsplive, "/live/0");
	rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H264, NULL, 0);
	rtsp_sync_video_ts(g_rtsp_session, rtsp_get_reltime(), rtsp_get_ntptime());

	// vi init
	vi_dev_init();
	vi_chn_init(0, width, height);

	// venc init
	RK_CODEC_ID_E enCodecType = RK_VIDEO_ID_AVC;
	venc_init(0, width, height, enCodecType);

	printf("venc init success\n");

	cv::Mat display_frame(DISP_HEIGHT, DISP_WIDTH, CV_8UC4, drm_fb_map);

	while (1)
	{
		uint64_t start_time = get_time_ms();
		// get vi frame
		h264_frame.stVFrame.u32TimeRef = H264_TimeRef++;
		h264_frame.stVFrame.u64PTS = TEST_COMM_GetNowUs();
		s32Ret = RK_MPI_VI_GetChnFrame(0, 0, &stViFrame, -1);
		if (s32Ret == RK_SUCCESS)
		{
			void *vi_data = RK_MPI_MB_Handle2VirAddr(stViFrame.stVFrame.pMbBlk);

			cv::Mat yuv420sp(height + height / 2, width, CV_8UC1, vi_data);
			cv::Mat rgb(480, 480, CV_8UC3, data);

			cv::cvtColor(yuv420sp, rgb, cv::COLOR_YUV420sp2RGB);
			cv::resize(rgb, frame, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);

			// letterbox
			cv::Mat letterboxImage = letterbox(frame);
			memcpy(rknn_app_ctx.input_mems[0]->virt_addr, letterboxImage.data, model_width * model_height * 3);
			inference_yolov5_model(&rknn_app_ctx, &od_results);

			// 转换为BGRA格式用于DRM显示
			cv::cvtColor(rgb, display_frame, cv::COLOR_RGB2RGBA);

			for (int i = 0; i < od_results.count; i++)
			{
				if (od_results.count >= 1)
				{
					object_detect_result *det_result = &(od_results.results[i]);

					sX = (int)(det_result->box.left);
					sY = (int)(det_result->box.top);
					eX = (int)(det_result->box.right);
					eY = (int)(det_result->box.bottom);
					mapCoordinates(&sX, &sY);
					mapCoordinates(&eX, &eY);

					printf("%s @ (%d %d %d %d) %.3f\n", coco_cls_to_name(det_result->cls_id),
						   sX, sY, eX, eY, det_result->prop);

					cv::rectangle(display_frame, cv::Point(sX, sY),
								  cv::Point(eX, eY),
								  cv::Scalar(0, 255, 0), 3);
					sprintf(text, "%s %.1f%%", coco_cls_to_name(det_result->cls_id), det_result->prop * 100);
					cv::putText(display_frame, text, cv::Point(sX, sY - 8),
								cv::FONT_HERSHEY_SIMPLEX, 1,
								cv::Scalar(0, 255, 0), 2);
				}
			}
			printf("display_frame cols,rows: %d %d \n", display_frame.cols, display_frame.rows);

			// memcpy(fb_map, display_frame.data, fb_size);
		}
		drmModeSetCrtc(drm_fd, drm_crtc->crtc_id, drm_fb_id, 0, 0, &drm_connector->connector_id, 1, &drm_connector->modes[0]); // memcpy(data, frame.data, width * height * 3);

		// encode H264
		RK_MPI_VENC_SendFrame(0, &h264_frame, -1);

		// rtsp
		s32Ret = RK_MPI_VENC_GetStream(0, &stFrame, -1);
		if (s32Ret == RK_SUCCESS)
		{
			if (g_rtsplive && g_rtsp_session)
			{
				// printf("len = %d PTS = %d \n",stFrame.pstPack->u32Len, stFrame.pstPack->u64PTS);
				void *pData = RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);
				rtsp_tx_video(g_rtsp_session, (uint8_t *)pData, stFrame.pstPack->u32Len,
							  stFrame.pstPack->u64PTS);
				rtsp_do_event(g_rtsplive);
			}
		}

		// rtsp

		// release frame
		s32Ret = RK_MPI_VI_ReleaseChnFrame(0, 0, &stViFrame);
		if (s32Ret != RK_SUCCESS)
		{
			RK_LOGE("RK_MPI_VI_ReleaseChnFrame fail %x", s32Ret);
		}
		s32Ret = RK_MPI_VENC_ReleaseStream(0, &stFrame);
		if (s32Ret != RK_SUCCESS)
		{
			RK_LOGE("RK_MPI_VENC_ReleaseStream fail %x", s32Ret);
		}
		// printf("after RK_MPI_VENC_ReleaseStream\n");
		memset(text, 0, 8);
		uint64_t end_time = get_time_ms();
		double total_time = (end_time - start_time) / 1000.0; // 转换为秒
		printf("cost %.2f seconds\n", total_time);
		printf("FPS: %.1f\n", 1000.0 / (get_time_ms() - start_time));
	}

	// Destory MB
	RK_MPI_MB_ReleaseMB(src_Blk);
	// Destory Pool
	RK_MPI_MB_DestroyPool(src_Pool);

	RK_MPI_VI_DisableChn(0, 0);
	RK_MPI_VI_DisableDev(0);

	SAMPLE_COMM_ISP_Stop(0);

	RK_MPI_VENC_StopRecvFrame(0);
	RK_MPI_VENC_DestroyChn(0);

	free(stFrame.pstPack);

	if (g_rtsplive)
		rtsp_del_demo(g_rtsplive);

	RK_MPI_SYS_Exit();

	// Release rknn model
	release_yolov5_model(&rknn_app_ctx);
	deinit_post_process();

	// 清理资源
	drm_cleanup();
	return 0;
}
