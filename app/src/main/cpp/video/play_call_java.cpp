//
// Created by Administrator on 2019/4/10.
//

#include "play_call_java.h"


play_call_java::play_call_java(JavaVM *vm, JNIEnv *env, jobject *obj) {
    this->play_javaVM = vm;
    this->play_jniEnv = env;
    this->play_jobj = env->NewGlobalRef(*obj);
    jclass j_clz = env->GetObjectClass(play_jobj);
    if (!j_clz) {
        LOGE("%s", "class失败");
        return;
    }

    play_j_method_id = env->GetMethodID(j_clz, "onCallPreared", "()V");
    play_time_rest_method_id = env->GetMethodID(j_clz, "getAudioTime", "(IJD)V");
}

play_call_java::~play_call_java() {

}

void play_call_java::onCallPrepared(int type) {
    if (type == MAIN_THREAD) {
        play_jniEnv->CallVoidMethod(play_jobj, play_j_method_id);
    } else if (type == CHILD_THREAD) {
        JNIEnv *_jniEnv;
        if (play_javaVM->AttachCurrentThread(&_jniEnv, NULL) != JNI_OK) {
            LOGE("%s", "获取子线程的jnienv失败");
            return;
        }
        _jniEnv->CallVoidMethod(play_jobj, play_j_method_id);
        play_javaVM->DetachCurrentThread();
    }
}

void play_call_java::onCallLoadAudioTime(int type, long time_base, double clock) {
    if (type == CHILD_THREAD) {
        JNIEnv *_jniEnv;
        if (play_javaVM->AttachCurrentThread(&_jniEnv, NULL) != JNI_OK) {
            LOGE("%s", "获取子线程的jnienv失败");
            return;
        }
        _jniEnv->CallVoidMethod(play_jobj, play_time_rest_method_id,type,time_base,clock);
        play_javaVM->DetachCurrentThread();
    } else if (type == MAIN_THREAD) {
        play_jniEnv->CallVoidMethod(play_jobj, play_time_rest_method_id,type,time_base,clock);
    }

}

