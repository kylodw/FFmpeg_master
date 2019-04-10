//
// Created by Administrator on 2019/4/10.
//

#ifndef FFMPEG_MASTER_PLAY_QUEUE_H
#define FFMPEG_MASTER_PLAY_QUEUE_H


#include "queue"
#include <pthread.h>
#include "play_status.h"

extern  "C"{
#include <libavcodec/avcodec.h>
};

class play_queue {
public:
    std::queue<AVPacket *> queue_packet;
    pthread_mutex_t mutex_p;
    pthread_cond_t cond_p;
    play_status *ps = NULL;
public:
    play_queue(play_status *status);

    ~play_queue();

    int putAvPacket(AVPacket *packet);
    int getAvPacket(AVPacket *packet);
    int getQueueSize();
};


#endif //FFMPEG_MASTER_PLAY_QUEUE_H
