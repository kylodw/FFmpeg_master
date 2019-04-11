//
// Created by Administrator on 2019/4/11.
//

#ifndef FFMPEG_MASTER_PLAY_VIDEO_H
#define FFMPEG_MASTER_PLAY_VIDEO_H


#include "play_queue.h"
#include "play_call_java.h"

extern "C" {
#include <libavcodec/avcodec.h>
};


class play_video {
public:
    int streamIndex = -1;
    AVCodecContext *codecContext = NULL;
    AVCodecParameters *codecpar = NULL;
    pthread_t play_thread;
    play_queue *queue = NULL;
    play_status *status = NULL;
    play_call_java *call_java = NULL;
    AVRational time_base;
    AVCodecContext *codec_context = NULL;
public:
    play_video(play_status *playStatus, play_call_java *callJava);
    ~play_video();


    void stream_source_play();
};


#endif //FFMPEG_MASTER_PLAY_VIDEO_H
