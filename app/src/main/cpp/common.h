//
// Created by kylodw on 2019/3/30.
//

#ifndef FFMPEG_MASTER_COMMON_H
#define FFMPEG_MASTER_COMMON_H

#endif //FFMPEG_MASTER_COMMON_H

#include <string>
#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <time.h>


using namespace std;


#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "这是loge", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "这是logi", format, ##__VA_ARGS__)


extern "C"{
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_VideoUtil_newdecode(JNIEnv *env, jclass type,
                                                                  jstring input_, jstring output_);
}

