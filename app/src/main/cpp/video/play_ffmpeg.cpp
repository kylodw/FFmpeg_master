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
                audio = new play_audio(status, call_java);
                audio->streamIndex = i;
                audio->total_duration = format_context->duration / AV_TIME_BASE;
                audio->time_base = format_context->streams[i]->time_base;
                audio->codecpar = format_context->streams[i]->codecpar;
//                duration = audio->total_duration;
            }
        } else if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video == NULL) {
                video = new play_video(status, call_java);
                video->streamIndex = i;
                video->codecpar = format_context->streams[i]->codecpar;
                video->time_base = format_context->streams[i]->time_base;
            }
        }
    }
    if (audio != NULL) {
        initCodecContext(audio->codecpar, &audio->codec_context);
    }
    if (video != NULL) {
        initCodecContext(video->codecpar, &video->codec_context);
    }
    if (call_java != NULL) {
        if (status != NULL && !status->exit) {
            call_java->onCallPrepared(CHILD_THREAD);
        } else {
            status->exit = true;
        }
    }
}

void play_ffmpeg::initCodecContext(AVCodecParameters *pParameters, AVCodecContext **pContext) {
    AVCodec *codec = avcodec_find_decoder(pParameters->codec_id);

    if (!codec) {
        LOGE("%s", "编码器未找到");
        return;
    }

    *pContext = avcodec_alloc_context3(codec);

    if (!(*pContext)) {
        LOGE("avcodec_alloc_context3失败");
        return;
    }

    ////将解码器中信息复制到上下文当中
    if (avcodec_parameters_to_context(*pContext, pParameters)) {
        LOGE("avcodec_parameters_to_context失败");
        return;
    }

    if (avcodec_open2(*pContext, codec, NULL) < 0) {
        LOGE("avcodec_open2失败");
        return;
    }
//    this->call_java->onCallPrepared(CHILD_THREAD);

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
    video->stream_source_play();
    decode_and_put_queue();
//    while (audio->queue->getQueueSize() > 0) {
//        AVPacket *pkt = av_packet_alloc();
//        audio->queue->getAvPacket(pkt);
//        av_packet_free(&pkt);
//        av_free(pkt);
//        pkt = NULL;
//    }
    LOGE("start解码完成");
}

play_ffmpeg::play_ffmpeg(play_status *p_ps, play_call_java *call_java, const char *url) {
    this->call_java = call_java;
    this->url = url;
    this->status = p_ps;
    pthread_mutex_init(&seek_mutex, NULL);
}

void play_ffmpeg::stop() {
    audio->stop_play();
}

void play_ffmpeg::play_pause() {
    audio->pause_play();
}

void play_ffmpeg::play_resume() {
    audio->resume_play();
}

void play_ffmpeg::play_audio_pcm_local() {
    if (audio == NULL) {
        audio = new play_audio(status, call_java);
    }
    audio->pcm_local_play();
}

void play_ffmpeg::play_audio_pcm_stream() {
    if (audio == NULL) {
        LOGE("audio为空");
        return;
    }
    audio->pcm_stream_play();
    video->stream_source_play();

    decode_and_put_queue();
    LOGE("play_audio_pcm_stream解码完成");
}

void play_ffmpeg::decode_and_put_queue() {
    int count = 0;
    while (status != NULL && !status->exit) {

        if (status->seeking) {
            av_usleep(1000 * 100);
            continue;//如果正在seeking
        }

        if (audio->queue->getQueueSize() > 40) {
            av_usleep(1000 * 100);
            continue;
        }
        int ret = 0;
        AVPacket *packet = av_packet_alloc();


//        pthread_mutex_lock(&seek_mutex);
        ret = av_read_frame(format_context, packet);
//        pthread_mutex_unlock(&seek_mutex);
        if (ret == 0) {
            if (packet->stream_index == audio->streamIndex) {
                count++;
                LOGE("第%d帧", count);
                audio->queue->putAvPacket(packet);
            } else if (packet->stream_index == video->streamIndex) {
                video->queue->putAvPacket(packet);

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
                    av_usleep(1000 * 100);
                    continue;
                } else {
                    status->exit = true;
                    break;
                }
            }
        }
    }
}

void play_ffmpeg::release() {
    audio->release_play();
}

void play_ffmpeg::seek(int64_t sec) {
    if (audio->total_duration < 0) {
        return;
    }
    status->seeking = true;
    if (sec >= 0 && sec <= audio->total_duration) {
        if (audio != NULL) {
            audio->queue->clearAvPacket();
            audio->audio_clock = 0;  //时间归零
//            audio->last_time = 0;

            pthread_mutex_lock(&seek_mutex);
            int64_t rel = sec * AV_TIME_BASE;
            avformat_seek_file(format_context, -1, INT64_MIN, rel, INT64_MAX, 0);
//            AV_NOPTS_VALUE
            pthread_mutex_unlock(&seek_mutex);
            status->seeking = false;

        }
    }


}



