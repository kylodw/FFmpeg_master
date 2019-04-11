//
// Created by Administrator on 2019/4/11.
//

#include "play_video.h"

play_video::play_video(play_status *playStatus, play_call_java *callJava) {
    this->status = playStatus;
    this->call_java = callJava;
    this->queue = new play_queue(playStatus);
}

play_video::~play_video() {

}

void *play_video_callback(void *data) {
    play_video *video = static_cast<play_video *>(data);
    while (video->status != NULL && !video->status->exit) {
        AVPacket *packet = av_packet_alloc();
        if (video->queue->getAvPacket(packet) == 0) {
            LOGE("获取到视频帧");
        }
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
}

void play_video::stream_source_play() {
    pthread_create(&play_thread, NULL, play_video_callback, this);

}
