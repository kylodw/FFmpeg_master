//
// Created by Administrator on 2019/4/10.
//

#ifndef FFMPEG_MASTER_JFFFMPEG_H
#define FFMPEG_MASTER_JFFFMPEG_H


#include "play_call_java.h"
#include "play_audio.h"
#include "play_video.h"
#include <pthread.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};


class play_ffmpeg {
public:
    play_call_java *call_java = NULL;
    const char *url = NULL;
    pthread_t decode_thread;
    int duration = 0;

    AVFormatContext *format_context = NULL;
    play_audio *audio = NULL;
    play_status *status;
    pthread_mutex_t seek_mutex;
    play_video *video = NULL;

public:
    play_ffmpeg(play_status *p_ps, play_call_java *call_java, const char *url);

    ~play_ffmpeg();

    void prepare();

    void decodeAudioThread();

    void start();

    void stop();

    void play_pause();

    void play_resume();

    void play_audio_pcm_local();

    void play_audio_pcm_stream();

    void decode_and_put_queue();

    void release();

    void seek(int64_t sec);

    void initCodecContext(AVCodecParameters *pParameters, AVCodecContext **pContext);
};


#endif //FFMPEG_MASTER_JFFFMPEG_H
