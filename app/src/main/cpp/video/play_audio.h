//
// Created by Administrator on 2019/4/10.
//

#ifndef FFMPEG_MASTER_PLAY_AUDIO_H
#define FFMPEG_MASTER_PLAY_AUDIO_H

#include "play_queue.h"

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
    AVPacket *packet=NULL;
    AVFrame *frame=NULL;
    int ret = -1;

    uint8_t *buffer = NULL;
    int data_size;//buffer size
public:
    play_audio(play_status *ps);

    ~play_audio();

    void thread_sampling();

    int resample_audio();
};


#endif //FFMPEG_MASTER_PLAY_AUDIO_H
