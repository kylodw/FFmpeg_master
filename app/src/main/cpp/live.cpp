//
// Created by Administrator on 2019/4/2.
//
#include "live.h"
#include "x264/x264.h"

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_sendVideo(JNIEnv *env, jobject instance,
                                                                      jbyteArray data_){
}

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_sendAudio(JNIEnv *env, jobject instance,
                                                                      jbyteArray data_, jint len){

}
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_startPush(JNIEnv *env, jobject instance,
                                                                      jstring url_){

}

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_stopPush(JNIEnv *env,
                                                                     jobject instance){

}

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_release(JNIEnv *env, jobject instance){

}

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_setVideoOptions(JNIEnv *env,
                                                                            jobject instance,
                                                                            jint width, jint height,
                                                                            jint bitrate,
                                                                            jint fps){

}

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_setAudioOptions(JNIEnv *env,
                                                                            jobject instance,
                                                                            jint sampleRateInHz,
                                                                            jint channel){

}
