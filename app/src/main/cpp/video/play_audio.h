//
// Created by Administrator on 2019/4/10.
//

#ifndef FFMPEG_MASTER_PLAY_AUDIO_H
#define FFMPEG_MASTER_PLAY_AUDIO_H

#include "play_queue.h"
#include "play_call_java.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
};


class play_audio {

public:
    int streamIndex = -1;
    AVCodecParameters *codecpar = NULL;
    AVCodecContext *codec_context = NULL;
    play_queue *queue;
    play_status *status;
    pthread_t sampling_thread;
    AVPacket *packet = NULL;
    AVFrame *frame = NULL;
    int ret = -1;
    AVRational time_base;
    int64_t total_duration;
    uint8_t *buffer = NULL;
    int data_size;//buffer size
    double audio_clock;

    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject;
    SLEnvironmentalReverbItf outputMixEnvReb;
    SLObjectItf pcmPlay;
    SLPlayItf pcmPlayerPlay;
    SLAndroidSimpleBufferQueueItf pcmBufferQueue;

    play_call_java  *callJava;
public:
    play_audio(play_status *ps,play_call_java  *callJava);

    ~play_audio();

    void thread_sampling();

    int resample_audio();

    void init_open_sles();

    void stop_play();

    void pause_play();

    void resume_play();

    void pcm_local_play();

    void pcm_stream_play();

    void release_play();



};


#endif //FFMPEG_MASTER_PLAY_AUDIO_H
