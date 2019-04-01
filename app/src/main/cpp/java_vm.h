//
// Created by kylodw on 2019/4/1.
//

#ifndef FFMPEG_MASTER_JAVA_VM_H
#define FFMPEG_MASTER_JAVA_VM_H

#include <jni.h>
#include <android/log.h>

#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "这是loge", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "这是logi", format, ##__VA_ARGS__)

JavaVM * javaVM;


extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGE("%s", "JNILOAD");
    javaVM = vm;
    //兼容AndroidSDk2.2之后
    return JNI_VERSION_1_4;
}

#endif //FFMPEG_MASTER_JAVA_VM_H
