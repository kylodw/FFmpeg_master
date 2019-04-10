//
// Created by Administrator on 2019/4/2.
//
#include <jni.h>


#ifndef FFMPEG_MASTER_LIVE_H
#define FFMPEG_MASTER_LIVE_H




extern "C" {
#include <rtmp/rtmp.h>
#include <pthread.h>
#include "queue_live.h"


JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_sendVideo(JNIEnv *env, jobject instance,
                                                                      jbyteArray data_);

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_sendAudio(JNIEnv *env, jobject instance,
                                                                      jbyteArray data_, jint len);
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_startPush(JNIEnv *env, jobject instance,
                                                                      jstring url_);

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_stopPush(JNIEnv *env,
                                                                     jobject instance);

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_release(JNIEnv *env, jobject instance);

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_setVideoOptions(JNIEnv *env,
                                                                            jobject instance,
                                                                            jint width, jint height,
                                                                            jint bitrate,
                                                                            jint fps);

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_setAudioOptions(JNIEnv *env,
                                                                            jobject instance,
                                                                            jint sampleRateInHz,
                                                                            jint channel);

void add_264_sequence_header(unsigned char *pps, unsigned char *sps, int len, int sps_len);
void add_264_body(uint8_t *payload, int i_payload);

void add_rtmp_packet(RTMPPacket *pPacket);
void *start_push(void *);
void add_aac_body(unsigned char *bitbuf, int byteslen);
void add_aac_sequence_header();

};


#endif //FFMPEG_MASTER_LIVE_H
