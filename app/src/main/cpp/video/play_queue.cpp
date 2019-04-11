//
// Created by Administrator on 2019/4/10.
//


#include "play_queue.h"

play_queue::play_queue(play_status *status) {
    this->ps = status;
    pthread_mutex_init(&mutex_p, NULL);
    pthread_cond_init(&cond_p, NULL);
}

play_queue::~play_queue() {
    pthread_mutex_destroy(&mutex_p);
    pthread_cond_destroy(&cond_p);
}

int play_queue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutex_p);
    size = queue_packet.size();
    pthread_mutex_unlock(&mutex_p);
    return size;
}

int play_queue::getAvPacket(AVPacket *packet) {
    pthread_mutex_lock(&mutex_p);
    while (ps != NULL && !ps->exit) {
        if (queue_packet.size() > 0) {
            AVPacket *pkt = queue_packet.front();
            //数据拷贝 pkt的数据拷贝到packet
            if (av_packet_ref(packet, pkt) == 0) {
                queue_packet.pop();
            }
            av_packet_free(&pkt);
            av_free(pkt);
            pkt = NULL;
            break;
        } else {
            pthread_cond_wait(&cond_p, &mutex_p);
        }

    }

    pthread_mutex_unlock(&mutex_p);
    return 0;
}

int play_queue::putAvPacket(AVPacket *packet) {
    pthread_mutex_lock(&mutex_p);
    queue_packet.push(packet);
    pthread_cond_signal(&cond_p);
    pthread_mutex_unlock(&mutex_p);
    return 0;
}

void play_queue::clearAvPacket() {
    //有可能释放资源时线程还在加锁
    pthread_cond_signal(&cond_p);

    pthread_mutex_lock(&mutex_p);
    while (!queue_packet.empty()) {
        AVPacket *packet = queue_packet.front();
        queue_packet.pop();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&mutex_p);

}
