//
// Created by Administrator on 2019/4/10.
//

#ifndef FFMPEG_MASTER_JFCALLJAVA_H
#define FFMPEG_MASTER_JFCALLJAVA_H
#define MAIN_THREAD 0
#define CHILD_THREAD 1

#include <jni.h>
#include <android/log.h>

#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "这是loge", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "这是logi", format, ##__VA_ARGS__)


class play_call_java {
public:
    JavaVM *play_javaVM = NULL;
    JNIEnv *play_jniEnv = NULL;
    jobject play_jobj;
    jmethodID play_j_method_id;
    jmethodID play_time_rest_method_id;

public:
    play_call_java(JavaVM *vm, JNIEnv *env, jobject *obj);

    ~play_call_java();

    void onCallPrepared(int type);

    void onCallLoadAudioTime(int type,long time_base, double clock);
};

#endif //FFMPEG_MASTER_JFCALLJAVA_H
