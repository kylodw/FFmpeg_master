//
// Created by Administrator on 2019/4/10.
//

#ifndef FFMPEG_MASTER_K_LOG_H
#define FFMPEG_MASTER_K_LOG_H

#include <android/log.h>
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "这是loge", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "这是logi", format, ##__VA_ARGS__)
#endif //FFMPEG_MASTER_K_LOG_H
