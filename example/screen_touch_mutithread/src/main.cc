#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "drm.h"
#include "touch.h"
#include "slider.h"
#include <string.h>

// 共享坐标变量
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

// 共享数据结构
typedef struct {
    int x;
    int y;
    bool updated;
    bool running;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} TouchData;

TouchData touch_data;

// TouchData touch_data = {0, 0, false, PTHREAD_MUTEX_INITIALIZER};
// 信号处理函数
void signal_handler(int signum) {
    if (signum == SIGINT) {
        pthread_mutex_lock(&touch_data.mutex);
        touch_data.running = false;
        pthread_cond_signal(&touch_data.cond);  // 唤醒等待的绘制线程
        pthread_mutex_unlock(&touch_data.mutex);
    }
}

// 触摸线程函数（优化版）
void* touch_thread(void* arg) {
    int x, y;
    while (1) {
        pthread_mutex_lock(&touch_data.mutex);
        if (!touch_data.running) {
            pthread_mutex_unlock(&touch_data.mutex);
            break;
        }
        pthread_mutex_unlock(&touch_data.mutex);

        if (touch_poll(&x, &y)) {  // 阻塞等待触摸事件
            pthread_mutex_lock(&touch_data.mutex);
            touch_data.x = x;
            touch_data.y = y;
            touch_data.updated = true;
            pthread_cond_signal(&touch_data.cond);  // 通知绘制线程有更新
            pthread_mutex_unlock(&touch_data.mutex);
        }
    }
    return NULL;
}

// 绘制线程函数（条件变量版）
void* draw_thread(void* arg) {
    pthread_mutex_lock(&touch_data.mutex);
    while (touch_data.running) {
        // 等待触摸更新或退出信号
        while (!touch_data.updated && touch_data.running) {
            pthread_cond_wait(&touch_data.cond, &touch_data.mutex);
        }

        if (!touch_data.running) break;

        if (touch_data.updated) {
            // 更新滑块位置并绘制
            slider_update(touch_data.x);
            memset(drm_fb_map, 0, drm_width * drm_height * 4);
            slider_draw(drm_fb_map);
            touch_data.updated = false;
        }
    }
    pthread_mutex_unlock(&touch_data.mutex);
    return NULL;
}
// void *touch_thread(void *arg)
// {
//     int x, y;
//     while (1)
//     {
//         if (touch_poll(&x, &y))
//         { // 阻塞等待触摸事件
//             pthread_mutex_lock(&touch_data.mutex);
//             touch_data.x = x;
//             touch_data.y = y;
//             touch_data.updated = true; // 标记数据已更新
//             pthread_mutex_unlock(&touch_data.mutex);
//         }
//     }
//     return NULL;
// }

// void *draw_thread(void *arg)
// {
//     while (1)
//     {
//         pthread_mutex_lock(&touch_data.mutex);
//         if (touch_data.updated)
//         {
//             // 更新滑块位置
//             slider_update(touch_data.x);

//             // 清空并重新绘制
//             memset(drm_fb_map, 0, drm_width * drm_height * 4);
//             slider_draw(drm_fb_map);

//             touch_data.updated = false; // 重置更新标记
//         }
//         pthread_mutex_unlock(&touch_data.mutex);
//         usleep(10000); // 控制绘制帧率
//     }
//     return NULL;
// }
int main()
{
    // 初始化共享数据结构
    touch_data.running = true;
    touch_data.updated = false;
    pthread_mutex_init(&touch_data.mutex, NULL);
    pthread_cond_init(&touch_data.cond, NULL);

    // 注册信号处理
    signal(SIGINT, signal_handler);

    // 初始化设备
    if (drm_setup() != 0)
    {
        fprintf(stderr, "DRM setup failed\n");
        return -1;
    }
    if (touch_setup() != 0)
    {
        fprintf(stderr, "Touch setup failed\n");
        drm_cleanup();
        return -1;
    }

    // 创建线程
    pthread_t touch_tid, draw_tid;
    if (pthread_create(&touch_tid, NULL, touch_thread, NULL) != 0)
    {
        perror("Failed to create touch thread");
        return -1;
    }
    if (pthread_create(&draw_tid, NULL, draw_thread, NULL) != 0)
    {
        perror("Failed to create draw thread");
        pthread_cancel(touch_tid);
        return -1;
    }

    // 等待线程结束
    pthread_join(touch_tid, NULL);
    pthread_join(draw_tid, NULL);

    // 清理资源
    pthread_mutex_destroy(&touch_data.mutex);
    pthread_cond_destroy(&touch_data.cond);
    touch_cleanup();
    drm_cleanup();
    return 0;
}
