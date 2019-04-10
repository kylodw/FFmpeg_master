//
// Created by Administrator on 2019/4/10.
//


#include "play_audio.h"

FILE *out_file = fopen("/storage/emulated/0/play_audio.pcm", "w");

play_audio::~play_audio() {

}

play_audio::play_audio(play_status *ps) {
    this->status = ps;
    queue = new play_queue(ps);
    buffer = static_cast<uint8_t *>(av_malloc(44100 * 2 * 2));

}

void *decode_play(void *data) {
    play_audio *audio = static_cast<play_audio *>(data);
    audio->resample_audio();
    pthread_exit(&audio->sampling_thread);
}

void play_audio::thread_sampling() {
    pthread_create(&sampling_thread, NULL, decode_play, this);
}

//重采样
int play_audio::resample_audio() {
    while (status != NULL && !status->exit) {
        packet = av_packet_alloc();
        if (queue->getAvPacket(packet) != 0) {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            continue;
        }
        ret = avcodec_send_packet(codec_context, packet);
        if (ret != 0) {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            continue;
        }

        frame = av_frame_alloc();
        ret = avcodec_receive_frame(codec_context, frame);

        if (ret == 0) {
            //声道个数和声道布局
            if (frame->channels > 0 && frame->channel_layout == 0) {
                frame->channel_layout = static_cast<uint64_t>(av_get_default_channel_layout(
                        frame->channels));
            } else if (frame->channels == 0 && frame->channel_layout > 0) {
                frame->channels = av_get_channel_layout_nb_channels(frame->channel_layout);
            }

            SwrContext *swr_context = NULL;
            swr_context = swr_alloc_set_opts(NULL,
                                             AV_CH_LAYOUT_STEREO,//立体声
                                             AV_SAMPLE_FMT_S16,  //采样率的位数
                                             frame->sample_rate, //输出的采样率
                                             frame->channel_layout,//声道布局
                                             static_cast<AVSampleFormat>(frame->format),//位数格式
                                             frame->sample_rate, NULL, NULL);
            if (!swr_context || swr_init(swr_context) < 0) {
                av_packet_free(&packet);
                av_free(packet);
                packet = NULL;

                av_frame_free(&frame);
                av_free(frame);
                frame = NULL;

                if (swr_context != NULL) {
                    swr_free(&swr_context);
                    swr_context = NULL;
                }
                continue;
            }

            int nbs = swr_convert(swr_context,
                                  &buffer,//转码pcm的数据大小
                                  frame->nb_samples,//输出的采样个数
                    (const uint8_t **)(frame->data),
                                  frame->nb_samples
            );

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nbs * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            fwrite(buffer, 1, data_size, out_file);

            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;

            av_frame_free(&frame);
            av_free(frame);
            frame = NULL;

            swr_free(&swr_context);
            swr_context = NULL;

        } else {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;

            av_frame_free(&frame);
            av_free(frame);
            frame = NULL;

            continue;
        }
    }
    fclose(out_file);
    return data_size;
}
