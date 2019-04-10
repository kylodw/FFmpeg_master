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
    audio->init_open_sles();
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
                                  (const uint8_t **) (frame->data),
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

static const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    play_audio *audio = (play_audio *) context;
    if (audio != NULL) {
        int buffer_size = audio->resample_audio();
        if (buffer_size > 0) {
            (*bf)->Enqueue(bf, audio->buffer,
                                              static_cast<SLuint32>(buffer_size));

        }
    } else {
    }
}

void play_audio::init_open_sles() {
    //第一步创建引擎
    slCreateEngine(&engineObject, 0, 0, 0, 0, 0);

    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);

    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二部，设置混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mrep[1] = {SL_BOOLEAN_FALSE};
    //创建混音器
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mrep);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvReb);

    (*outputMixEnvReb)->SetEnvironmentalReverbProperties(outputMixEnvReb, &reverbSettings);

    //创建播放器
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                                   SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource dataSource = {&android_queue, &format_pcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    SLDataSink dataSink = {&outputMix, NULL};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlay, &dataSource, &dataSink, 1, ids, req);
    (*pcmPlay)->Realize(pcmPlay, SL_BOOLEAN_FALSE);
// 7.获得缓冲区队列接口Buffer Queue Interface
    //通过缓冲区队列接口对缓冲区进行排序播放
    (*pcmPlay)->GetInterface(pcmPlay, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //设置缓冲队列和回调函数
    //SLBufferQueueItf self,
//    slBufferQueueCallback callback = NULL;
// 8.注册音频播放器回调函数
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
    //9.获取Play Interface通过对SetPlayState函数来启动播放音乐
    //一旦播放器被设置为播放状态，该音频播放器开始等待缓冲区排队就绪
    (*pcmPlay)->GetInterface(pcmPlay, SL_IID_PLAY, &pcmPlayerPlay);
    //设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);

    //10.开始，让第一个缓冲区入队
    pcmBufferCallBack(pcmBufferQueue, this);

}
