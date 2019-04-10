//
// Created by Administrator on 2019/4/10.
//

#include "play_ffmpeg.h"


play_ffmpeg::~play_ffmpeg() {

}

void play_ffmpeg::decodeAudioThread() {
    av_register_all();
    avformat_network_init();

    format_context = avformat_alloc_context();

    if (avformat_open_input(&format_context, url, NULL, NULL) != 0) {
        LOGE("%s", "打开文件失败");
        return;
    }

    if (avformat_find_stream_info(format_context, NULL) < 0) {
        LOGE("%s", "文件没有发现流");
        return;
    }
    for (int i = 0; i < format_context->nb_streams; i++) {
        if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new play_audio(status);
                audio->streamIndex = i;
                audio->codecpar = format_context->streams[i]->codecpar;
            }
        }
    }
    AVCodec *codec = avcodec_find_decoder(audio->codecpar->codec_id);

    if (!codec) {
        LOGE("%s", "编码器未找到");
        return;
    }

    audio->codec_context = avcodec_alloc_context3(codec);

    if (!audio->codec_context) {
        LOGE("avcodec_alloc_context3失败");
        return;
    }

    ////将解码器中信息复制到上下文当中
    if (avcodec_parameters_to_context(audio->codec_context, audio->codecpar)) {
        LOGE("avcodec_parameters_to_context失败");
        return;
    }

    if (avcodec_open2(audio->codec_context, codec, NULL) < 0) {
        LOGE("avcodec_open2失败");
        return;
    }
    this->call_java->onCallPrepared(CHILD_THREAD);

}

void *decodeFFmpeg(void *data) {
    play_ffmpeg *pf = static_cast<play_ffmpeg *>(data);
    pf->decodeAudioThread();
    pthread_exit(&pf->decode_thread);

}

void play_ffmpeg::prepare() {
    pthread_create(&decode_thread, NULL, decodeFFmpeg, this);
}

void play_ffmpeg::start() {
    if (audio == NULL) {
        LOGE("AUDIO为NULL");
        return;
    }
    audio->thread_sampling();
    int count = 0;
    while (status != NULL && !status->exit) {
        AVPacket *packet = av_packet_alloc();
        if (av_read_frame(format_context, packet) == 0) {
            if (packet->stream_index == audio->streamIndex) {
                count++;
                LOGE("第%d帧", count);
                audio->queue->putAvPacket(packet);
            } else {
                av_packet_free(&packet);
                av_free(packet);
                packet = NULL;
            }
        } else {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            while (status != NULL && !status->exit) {
                if (audio->queue->getQueueSize() > 0) {
                    continue;
                } else {
                    status->exit = true;
                    break;
                }
            }
        }
    }
//    while (audio->queue->getQueueSize() > 0) {
//        AVPacket *pkt = av_packet_alloc();
//        audio->queue->getAvPacket(pkt);
//        av_packet_free(&pkt);
//        av_free(pkt);
//        pkt = NULL;
//    }
    LOGE("解码完成");
}

play_ffmpeg::play_ffmpeg(play_status *p_ps, play_call_java *call_java, const char *url) {
    this->call_java = call_java;
    this->url = url;
    this->status = p_ps;
}

