//
// Created by kylodw on 2019/4/1.
//

#ifndef FFMPEG_MASTER_JAVA_VM_H
#define FFMPEG_MASTER_JAVA_VM_H

#include <jni.h>
#include <android/log.h>

#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "这是loge", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "这是logi", format, ##__VA_ARGS__)

extern JavaVM * javaVM;




#endif //FFMPEG_MASTER_JAVA_VM_H
