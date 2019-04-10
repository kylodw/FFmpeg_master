//
// Created by Administrator on 2019/4/10.
//

#ifndef FFMPEG_MASTER_JFFFMPEG_H
#define FFMPEG_MASTER_JFFFMPEG_H


#include "play_call_java.h"
#include "play_audio.h"
#include <pthread.h>

extern "C" {
#include <libavformat/avformat.h>
};


class play_ffmpeg {
public:
    play_call_java *call_java = NULL;
    const char *url = NULL;
    pthread_t decode_thread;


    AVFormatContext *format_context = NULL;
    play_audio *audio = NULL;
    play_status *status;
public:
    play_ffmpeg(play_status *p_ps,play_call_java *call_java, const char *url);

    ~play_ffmpeg();

    void prepare();

    void decodeAudioThread();

    void start();
};


#endif //FFMPEG_MASTER_JFFFMPEG_H
