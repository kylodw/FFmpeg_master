//
// Created by Administrator on 2019/4/1.
//

#include "queue.h"
#include "video_play.h"
#include "java_vm.h"

//AVStream读取视频音频数据   压缩数据packet
//stream数组
//AVFormatContext 封装格式上下文->AVStream[0]视频流->AVCodecContext解码器上下文->AVCodec解码器
//AVStream[1]
//todo 这里需要改变
#define PACKET_QUEUE_SIZE 500
#define AUDIO_TIME_ADJUST_US -20000011
#define MIN_SlEEP_TIME_US  130011
//100011
JNIEXPORT void init_input_from_context(Player *player, const char *input_p_s) {
    av_register_all();

    AVFormatContext *format_context = avformat_alloc_context();

    int open_result_code = avformat_open_input(&format_context, input_p_s, NULL, NULL);
    if (open_result_code != 0) {
        LOGE("%s", "打开输入视频文件失败");
        return;
    }
    if (avformat_find_stream_info(format_context, NULL) < 0) {
        LOGE("%s", "获取视频信息失败");
        return;
    }
    player->capture_stream_no = format_context->nb_streams;
    LOGE("流的个数%d", player->capture_stream_no);
    //解完封装  根据索引位置去解码   找到视频AVStream的索引位置
    int i = 0;
    //多少个视频数据
    for (; i < format_context->nb_streams; i++) {
        //根据类型判断，是否是视频流
        //如果需要字幕 也需要一个AVStream流
        if (format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            player->video_stream_index = i;
        } else if (format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            player->audio_stream_index = i;
        } else {
            LOGE("%s", "没有找到视频流");
        }
    }
    player->input_format_context = format_context;
}

JNIEXPORT void init_codec_context(Player *player, int stream_index) {
    AVFormatContext *fm_cxt = player->input_format_context;
    //4，解码器  解码上下文  AVCodecContext 保存了编解码的相关信息
    AVCodecContext *codecContext = fm_cxt->streams[stream_index]->codec;
    //根据id找到对应的解码器
    AVCodec *pCodec = avcodec_find_decoder(codecContext->codec_id);
    if (pCodec == NULL) {
        LOGE("%s", "无法解码");
        return;
    }
    int open_codec_result_code = avcodec_open2(codecContext, pCodec, NULL);
    if (open_codec_result_code < 0) {
        LOGE("%s", "找不到解码或者解码器无法打开");
        return;
    }
    player->input_codec_context[stream_index] = codecContext;

}

int64_t player_get_current_video_time(Player *player) {
    int64_t current_time = av_gettime(); //标准时间
    return current_time - player->start_time;
}

void player_wait_for_frame(Player *player, int64_t stream_time, int stream_no) {
    pthread_mutex_lock(&player->mutex);
    for (;;) {
        int64_t current_video_time = player_get_current_video_time(player);
        int64_t sleep_time = stream_time - current_video_time;
        //  99384
        if (sleep_time < -30000011) {
            int64_t new_value = player->start_time - sleep_time;
            player->start_time = new_value;
            pthread_cond_broadcast(&player->cond);
        }
        if (sleep_time <= MIN_SlEEP_TIME_US) {
            break;
        }
        if (sleep_time > 50000011) {
            sleep_time = 50000011;
        }
        //等待指定时长
        struct timespec abstime;
        struct timeval now;
        long timeout_ms = static_cast<long>(sleep_time / 100011); // wait time 100ms
        gettimeofday(&now, NULL);
        long nsec = now.tv_usec * 1000 + (timeout_ms % 1000) * 1000000;
        abstime.tv_sec=now.tv_sec + nsec / 1000000000 + timeout_ms / 1000;
        abstime.tv_nsec=nsec % 1000000000;
        pthread_cond_timedwait(&player->cond, &player->mutex, &abstime);

//        struct timespec *_timespec;
//        _timespec->tv_nsec = (long) (sleep_time / 100011);
//        int timeout_ret = pthread_cond_timedwait(&player->cond, &player->mutex, _timespec);
//        LOGE("player_wait_for_frame[%d] finish时间:%d", stream_no, timeout_ret);
    }
    pthread_mutex_unlock(&player->mutex);
}

void decode_video(Player *player, AVPacket *avPacket) {
    AVFormatContext *formatContext = player->input_format_context;
    AVStream *stream = formatContext->streams[player->video_stream_index];
    AVCodecContext *codecContext = player->input_codec_context[player->video_stream_index];
    int  got_frame= 0;
    //像素数据（解码数据）
    AVFrame *avFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();
//    AVStream stream;
//    stream.duration*stream.time_base; 真正的时长
//    avFrame->pts  //时间戳

    //绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;

     avcodec_decode_video2(codecContext, avFrame, &got_frame, avPacket);
    //非零，正在解码
    if (got_frame) {
        //AudioTrack.writePCM数据
        //lock
        //设置缓冲区的属性 宽高像素格式
        //最终缓冲区的数据  surface_view
        ANativeWindow_setBuffersGeometry(player->nativeWindow, codecContext->width,
                                         codecContext->height, WINDOW_FORMAT_RGBA_8888);
        ANativeWindow_lock(player->nativeWindow, &outBuffer, NULL);
        //fix buffer  要转换成RGBA_8888  YUV
        //设置yuv缓冲区属性宽高等等，像素格式
        //操作的缓冲区 outBuffer
        //YUV的画面转成RBG 绘制到buffer上，解锁之后，buffer传递给surface
        //关联了surface的缓冲区
        avpicture_fill((AVPicture *) (rgbFrame), (uint8_t *) (outBuffer.bits), AV_PIX_FMT_RGBA,
                       codecContext->width, codecContext->height);

        //YUV_RGBA8888
        int result = I420ToARGB(avFrame->data[0], avFrame->linesize[0],
                                avFrame->data[2],
                                avFrame->linesize[2],//这一行移到1的上面可以修改人脸为绿色的bug，色度对调 示例程序是这样的
                                avFrame->data[1], avFrame->linesize[1],
                                rgbFrame->data[0], rgbFrame->linesize[0],
                                codecContext->width, codecContext->height
        );
//            av_read_frame()
        //计算延迟
//
        int64_t pts = av_frame_get_best_effort_timestamp(avFrame);
        //转换(不同时间基转换)
        int64_t time = av_rescale_q(pts, stream->time_base, AV_TIME_BASE_Q);
        player_wait_for_frame(player, time, player->video_stream_index);
        //unlock 解锁
        //绘制完成
        ANativeWindow_unlockAndPost(player->nativeWindow);
    }

    av_frame_free(&avFrame);
    av_frame_free(&rgbFrame);


}

/**
 * 音视频都在这里解码
 * @param arg
 * @return
 */
void *decode_data(void *arg) {
    DecodeData *decode_data = (DecodeData *) (arg);
    //6. 一帧一帧读取压缩的数据AVPacket
    //这里为啥要开辟啊
    //压缩数据
    Player *player = decode_data->player;
    int stream_index = decode_data->stream_index;
    //根据index获取对应的队列
    Queue *queue = player->packets[stream_index];

    int video_frame_count = 0, audio_frame_count = 0;

    //不断的读取压缩数据
    for (;;) {
        pthread_mutex_lock(&player->mutex);
        //消费AVPacket
        AVPacket *packet = (AVPacket *) (queue_pop(queue, &player->mutex, &player->cond));
        pthread_mutex_unlock(&player->mutex);
        if (packet->stream_index == player->video_stream_index) {
            decode_video(player, packet);
        } else if (packet->stream_index == player->audio_stream_index) {
            decode_audio(player, packet);
        }

    }
    return NULL;
}

void decode_video_prepare(JNIEnv *env, Player *player, jobject surface) {
    //native 绘制
    player->nativeWindow = ANativeWindow_fromSurface(env, surface);
}

int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt) {
    int ret;
    *got_frame = 0;
    if (pkt) {
        ret = avcodec_send_packet(avctx, pkt);
        // In particular, we don't expect AVERROR(EAGAIN), because we read all
        // decoded frames with avcodec_receive_frame() until done.
        if (ret < 0 && ret != AVERROR_EOF)
            return ret;
    }
    ret = avcodec_receive_frame(avctx, frame);
    if (ret < 0 && ret != AVERROR(EAGAIN))
        return ret;
    if (ret >= 0)
        *got_frame = 1;

    return 0;
}

/**
 * 初始化class文件
 */
void jni_audio_init(JNIEnv *env, jobject jthiz, Player *player) {
    //JasonPlayer
    jclass player_class = env->GetObjectClass(jthiz);
    //AudioTrack对象
    jmethodID create_audio_track_mid = env->GetMethodID(player_class, "createAudioTrack",
                                                        "(II)Landroid/media/AudioTrack;");
    jobject audio_track = env->CallObjectMethod(jthiz, create_audio_track_mid,
                                                player->out_sample_rate,
                                                player->out_channel_nb);
    if (audio_track == NULL) {
        LOGE("%s", "audio_track为空");
    }
    //调用AudioTrack.play方法
    jclass audio_track_class = env->GetObjectClass(audio_track);

    jmethodID audio_track_play_mid = env->GetMethodID(audio_track_class, "play", "()V");
    env->CallVoidMethod(audio_track, audio_track_play_mid);

    //AudioTrack.write
    jmethodID audio_track_write_mid = env->GetMethodID(audio_track_class, "write", "([BII)I");

    player->audio_track = env->NewGlobalRef(audio_track);
    player->audio_track_write_mid = audio_track_write_mid;


}

int position = 0;

//解码在子线程里面
void decode_audio(Player *player, AVPacket *avPacket) {
    AVFormatContext *formatContext = player->input_format_context;
    AVStream *stream = formatContext->streams[player->video_stream_index];
    AVCodecContext *codec_ctx = player->input_codec_context[player->audio_stream_index];
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //16bit 44100 PCM 数据(重采样缓冲区)
    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);
    int got_frame = 0, index = 0, ret;
    ret = decode(codec_ctx, frame, &got_frame, avPacket);
    LOGE("%d------%d", ret, got_frame);
    if (ret >= 0) {
        LOGE("%s", "解码完成");
    } else {
        LOGE("%s", "解码失败");
    }
    if (got_frame > 0) {
        LOGE("解码got_frame：%d", position++);
        swr_convert(player->swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE,
                    (const uint8_t **) frame->data, frame->nb_samples);
//        //获取sample的size
        int out_buffer_size = av_samples_get_buffer_size(NULL, player->out_channel_nb,
                                                         frame->nb_samples, player->out_sample_fmt,
                                                         1);

        //转换(不同时间基转换)
        int64_t pts = avPacket->pts;
//        if (pts != AV_NOPTS_VALUE) {
//            player->audio_clock = av_rescale_q(pts, stream->time_base, AV_TIME_BASE_Q);
//
//            //for test
//            int64_t current_video_time = player_get_current_video_time(player);
//            int64_t sleep_time = player->audio_clock - current_video_time;
//            LOGE("sleep_time%ld",(long)sleep_time);
//
//            player_wait_for_frame(player, player->audio_clock ,
//                                  player->audio_stream_index);
//        }
//        // 一个pcm文件  fwrite(out_buffer, 1, out_buffer_size, fp_pcm);
        JavaVM *javaVM = player->javaVM;
        JNIEnv *env;
        javaVM->AttachCurrentThread(&env, NULL);
//        //out_buffer缓冲区数据，转成byte数组
        jbyteArray audio_sample_array = env->NewByteArray(out_buffer_size);
        jbyte *sample_bytep = env->GetByteArrayElements(audio_sample_array, NULL);
//        //out_buffer的数据复制到sampe_bytep
        memcpy(sample_bytep, out_buffer, out_buffer_size);
        env->ReleaseByteArrayElements(audio_sample_array, sample_bytep, 0);
//
//        //AudioTrack.write PCM数据
        env->CallIntMethod(player->audio_track, player->audio_track_write_mid,
                           audio_sample_array, 0, out_buffer_size);
        env->DeleteLocalRef(audio_sample_array);
//        env->DeleteGlobalRef(player->audio_track);
        javaVM->DetachCurrentThread();
//        usleep(1000 * 16);
    }
    av_frame_free(&frame);
}

JNIEXPORT void decode_audio_prepare(JNIEnv *pEnv, Player *player) {
    SwrContext *swrCtx = swr_alloc();
    AVCodecContext *codecCtx = player->input_codec_context[player->audio_stream_index];
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecCtx->sample_rate;
    //输出采样率
    int out_sample_rate = in_sample_rate;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t in_ch_layout = codecCtx->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrCtx,
                       out_ch_layout, out_sample_fmt, out_sample_rate,
                       in_ch_layout, in_sample_fmt, in_sample_rate,
                       0, NULL);
    swr_init(swrCtx);

    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    player->swrCtx = swrCtx;
    player->in_sample_fmt = in_sample_fmt;
    player->out_sample_fmt = out_sample_fmt;
    player->in_sample_rate = in_sample_rate;
    player->out_sample_rate = out_sample_rate;
    player->out_channel_nb = out_channel_nb;


}

/**
 * 开辟AVPacket内存空间
 */
void *player_fill_packet() {
    AVPacket *packet = (AVPacket *) (malloc(sizeof(AVPacket)));
    return packet;
}

/**
 * 开辟
 */
void player_alloc_queues(Player *player) {
    int i = 0;
    if (player->capture_stream_no == 0) {
        LOGE("流为空%d", player->capture_stream_no);
        return;
    }
    for (; i < player->capture_stream_no; ++i) {
        Queue *queue = queue_init(PACKET_QUEUE_SIZE, (queue_fill_func) player_fill_packet);
        player->packets[i] = queue;


    }
}

void *packet_free_func(void *args) {
    AVPacket *packet = static_cast<AVPacket *>(args);
    av_packet_unref(packet);
    return 0;
}

/**
 * 生产者
 * @param arg
 * @return
 */
void *player_read_from_stream(void *arg) {
    Player *player = static_cast<Player *>(arg);
    int index = 0;
    int ret = 0;
    //栈内存上保存一个AVPacket
    AVPacket packet, *pkt = &packet;
    for (;;) {
        ret = av_read_frame(player->input_format_context, pkt);
        LOGE("ret的值：%d", ret);
        if (ret < 0) {
            break;
        }
        Queue *queue = player->packets[pkt->stream_index];
        if (queue == NULL) {
            LOGE("%s", "queue为null");
        }
        LOGE("packet.stream_index:%d", pkt->stream_index);
//        queue_free(queue, (packet_free_func));
        //先止痛剂视频的
//        if (pkt->stream_index == player->video_stream_index && index++ >PACKET_QUEUE_SIZE - 5) {
//            for (;;) {
//                sleep(5);
//            }
//        }
        pthread_mutex_lock(&player->mutex);
        //push
        AVPacket *packet_data = (AVPacket *) (queue_push(queue, &player->mutex, &player->cond));
        //这里进行拷贝，间接赋值
        *packet_data = packet;
        pthread_mutex_unlock(&player->mutex);
    }
    return NULL;
}


extern "C" void JNICALL
Java_com_example_administrator_ffmpeg_1master_VideoUtil_play(JNIEnv *env, jobject instance,
                                                             jstring input_, jobject surface) {
    const char *input_p = env->GetStringUTFChars(input_, NULL);
    Player *player = (Player *) (malloc(sizeof(Player)));
    //javavm全局要唯一，不能多个源文件同时存在
    if (javaVM == NULL) {
        LOGE("%s", "java虚拟机为空");
        return;
    }
    player->javaVM = javaVM;
    if (player->javaVM == NULL) {
        LOGE("%s", "player-javavm为null");
    }

    //进行初始化
    init_input_from_context(player, input_p);

    int video_stream_index_i = player->video_stream_index;
    int audio_stream_index_i = player->audio_stream_index;
    init_codec_context(player, video_stream_index_i);
    init_codec_context(player, audio_stream_index_i);
    decode_video_prepare(env, player, surface);
    decode_audio_prepare(env, player);

    jni_audio_init(env, instance, player);
    player_alloc_queues(player);
    //传进去改变值
    pthread_mutex_init(&player->mutex, NULL);
    pthread_cond_init(&player->cond, NULL);
    LOGE("%s", "进入到线程创建阶段");
    pthread_create(&(player->thread_read_from_stream), NULL, player_read_from_stream,
                   (void *) player);
    sleep(5);
    player->start_time = 0;

    DecodeData data1 = {player, video_stream_index_i}, *decode_data_1 = &data1;
    pthread_create(&(player->decode_threads[video_stream_index_i]), NULL, decode_data,
                   (void *) decode_data_1);
    DecodeData data2 = {player, audio_stream_index_i}, *decode_data_2 = &data2;
    pthread_create(&(player->decode_threads[audio_stream_index_i]), NULL, decode_data,
                   (void *) decode_data_2);

    pthread_join(player->thread_read_from_stream, NULL);
    pthread_join(player->decode_threads[video_stream_index_i], NULL);
    pthread_join(player->decode_threads[audio_stream_index_i], NULL);
    //    ANativeWindow_release(nativeWindow);
//    av_frame_free(&avFrame);
//    avformat_free_context(player->input_format_context);
//    env->ReleaseStringUTFChars(input_, input_p);
//
//    free(player);

}