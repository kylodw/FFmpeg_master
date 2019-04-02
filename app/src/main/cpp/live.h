//
// Created by Administrator on 2019/4/2.
//
#include <jni.h>

#ifndef FFMPEG_MASTER_LIVE_H
#define FFMPEG_MASTER_LIVE_H
extern "C" {
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_sendVideo(JNIEnv *env, jobject instance,
                                                                      jbyteArray data_);

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_sendAudio(JNIEnv *env, jobject instance,
                                                                      jbyteArray data_, jint len);
};
extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_startPush(JNIEnv *env, jobject instance,
                                                                      jstring url_);
extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_stopPush(JNIEnv *env,
                                                                     jobject instance);

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_release(JNIEnv *env, jobject instance);

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_setVideoOptions(JNIEnv *env,
                                                                            jobject instance,
                                                                            jint width, jint height,
                                                                            jint bitrate,
                                                                            jint fps);

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_setAudioOptions(JNIEnv *env,
                                                                            jobject instance,
                                                                            jint sampleRateInHz,
                                                                            jint channel);

#endif //FFMPEG_MASTER_LIVE_H
