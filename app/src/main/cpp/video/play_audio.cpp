//
// Created by Administrator on 2019/4/10.
//


#include "play_audio.h"
#include "../k_log.h"
#include "play_call_java.h"

#define MAX_BUFF_SIZE 1024*1024
FILE *out_file = fopen("/storage/emulated/0/play_audio.pcm", "w");
//要不要写入文件  true表示写入文件  false表示直接用
bool is_call = true;

play_audio::~play_audio() {

}

play_audio::play_audio(play_status *ps, play_call_java *callJava) {
    this->status = ps;
    this->callJava = callJava;
    queue = new play_queue(ps);
    buffer = static_cast<uint8_t *>(av_malloc(MAX_BUFF_SIZE));

}

void *decode_play(void *data) {
    play_audio *audio = static_cast<play_audio *>(data);
    if (is_call) {
        LOGE("进入文件流");
        audio->resample_audio();
    } else {
        LOGE("进入opensles");
        audio->init_open_sles();
    }

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
//        ret = av_seek_frame(format_context, -1, 20 * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
//        if (ret >= 0) {
//            LOGE("跳转成功");
//        } else {
//            LOGE("跳转失败");
        ret = avcodec_send_packet(codec_context, packet);
        if (ret != 0) {
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
            continue;
        }

        frame = av_frame_alloc();
        ret = avcodec_receive_frame(codec_context, frame);
        //时间转化
        double now_time = frame->pts * av_q2d(time_base);
        if (now_time < audio_clock) {
            now_time = audio_clock;
        }
        audio_clock = now_time;
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
            data_size = av_samples_get_buffer_size(NULL, out_channels,
                                                   nbs, AV_SAMPLE_FMT_S16, 1);
            audio_clock += data_size / (double) (MAX_BUFF_SIZE);
            //回调给java层进度
            callJava->onCallLoadAudioTime(CHILD_THREAD, static_cast<long>(total_duration),
                                          audio_clock);
//            LOGE("视频总长度：%jd ,每次的进度：%f", total_duration, audio_clock);
//            data_size = nbs * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            if (is_call) {
                fwrite(buffer, 1, data_size, out_file);
            }


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
    if (is_call) {
        fclose(out_file);
    }
    return data_size;
}

static const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {

    if (is_call) {
        static FILE *fp = NULL;
        static char *buf = NULL;
        if (!buf) {
            buf = new char[1024 * 1024];
        }
        LOGE("进入callback");
        if (!fp) {
            fp = fopen("/storage/emulated/0/rawtest.pcm", "rb");
        }
        if (!fp) {
            LOGE("打开文件失败");
        } else {
            LOGE("打开文件成功");
        }
        if (feof(fp) == 0) {
            int len = fread(buf, 1, 1024, fp);
            LOGE("进度:%d", len);
            if (len > 0) {
                (*bf)->Enqueue(bf, buf, len);
            }
        }
    } else {
        play_audio *audio = (play_audio *) context;
        if (audio != NULL) {
            int buffer_size = audio->resample_audio();
//            LOGE("buff_size: %d", buffer_size);
            if (buffer_size > 0) {
                int result = (*bf)->Enqueue(bf, audio->buffer,
                                            static_cast<SLuint32>(buffer_size));
                if (result != SL_RESULT_SUCCESS) {
                    LOGE("入队失败");
                } else {
                    LOGE("入队成功");
                }
            }
        } else {
        }
    }

}

void play_audio::init_open_sles() {

    //第一步创建引擎
    SLresult result;
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("创建引擎失败");
        return;
    }
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("实例化SL_BOOLEAN_FALSE等待对象创建失败");
        return;
    }

    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("获取引擎接口失败");
        return;
    }


    //第二部，设置混音器
//    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
//    const SLboolean mrep[1] = {SL_BOOLEAN_FALSE};
    //创建混音器
//    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mrep);
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, 0, 0);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("创建混音器失败");
        return;
    }


    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("实例化混音器失败");
        return;
    }

    //输出
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};

    SLDataSink dataSink = {&outputMix, NULL};


//    (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvReb);
//
//    (*outputMixEnvReb)->SetEnvironmentalReverbProperties(outputMixEnvReb, &reverbSettings);

    //创建缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            10};
    //音频格式配置
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2,//通道数
                                   SL_SAMPLINGRATE_44_1, //采样率
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,//bits
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//声道
                                   SL_BYTEORDER_LITTLEENDIAN};//字节序
    //播放器使用的结构体
    SLDataSource dataSource = {&android_queue, &format_pcm};



    //播放器对象
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlay, &dataSource, &dataSink,
                                                sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("创建播放器失败");
        return;
    } else {
        LOGE("创建播放器成功");
    }


    (*pcmPlay)->Realize(pcmPlay, SL_BOOLEAN_FALSE);

    result = (*pcmPlay)->GetInterface(pcmPlay, SL_IID_PLAY, &pcmPlayerPlay);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("获取播放器接口失败");
        return;
    } else {
        LOGE("获取播放器接口成功");
    }


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

    //设置播放状态
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);

    //10.开始，让第一个缓冲区入队
    pcmBufferCallBack(pcmBufferQueue, this);

}

void play_audio::stop_play() {
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    queue->clearAvPacket();
}

void play_audio::pause_play() {
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
}

void play_audio::resume_play() {
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
}

void *local_pcm(void *data) {
    play_audio *audio = static_cast<play_audio *>(data);
    audio->init_open_sles();
    pthread_exit(&audio->sampling_thread);
}

void play_audio::pcm_local_play() {
    pthread_create(&sampling_thread, NULL, local_pcm, this);

}

void play_audio::pcm_stream_play() {
    is_call = false;
    thread_sampling();

}

void play_audio::release_play() {
    if (queue != NULL) {
        delete (queue);
        queue = NULL;
    }

    if (pcmPlay != NULL) {
        (*pcmPlay)->Destroy(pcmPlay);
        pcmPlay = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
    }

    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvReb = NULL;
    }

    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    if (codec_context != NULL) {
        avcodec_close(codec_context);
        avcodec_free_context(&codec_context);
        codec_context = NULL;
    }

    if (status != NULL) {
        status = NULL;
    }


}
