//
// Created by Administrator on 2019/4/1.
//
#include "common.h"
#include <android/native_window.h>
#include <pthread.h>
#include <android/native_window_jni.h>
#include <unistd.h>


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libyuv.h>

void init_codec_context(struct Player *player, int stream_index);

void init_input_from_context(struct Player *player, const char *input_p_s);

void *decode_data(void *arg);

void decode_video_prepare(JNIEnv *env, struct Player *player, jobject surface);
void decode_video(struct Player *player, AVPacket *pPacket);
void decode_audio(struct Player *player, AVPacket *avPacket);

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_VideoUtil_play(JNIEnv *env, jobject instance,
                                                             jstring input_, jobject surface);
}
using namespace libyuv;
//nb_streams  音频流，视频流，字幕
#define MAX_STREAM 2
#define MAX_AUDIO_FRME_SIZE 48000 * 4

//AVStream读取视频音频数据   压缩数据packet
//stream数组
//AVFormatContext 封装格式上下文->AVStream[0]视频流->AVCodecContext解码器上下文->AVCodec解码器
//AVStream[1]
struct Player {

    JavaVM *javaVM;
    //封装格式上下文
    AVFormatContext *input_format_context;
    int video_stream_index;
    int audio_stream_index;
    //解码器上下文数组
    AVCodecContext *input_codec_context[MAX_STREAM];
    pthread_t decode_threads[MAX_STREAM];
    ANativeWindow *nativeWindow;

    enum AVSampleFormat in_sample_fmt;
    enum AVSampleFormat out_sample_fmt;
    int in_sample_rate;
    int out_sample_rate;
    int out_channel_nb;
    SwrContext *swrCtx;

    jobject audio_track;
    jmethodID audio_track_write_mid;
};


/**
 *初始化
 */
void decode_audio_prepare(JNIEnv *pEnv, Player *pPlayer);

JNIEXPORT void init_input_from_context(struct Player *player, const char *input_p_s) {
    av_register_all();

    AVFormatContext *format_context = avformat_alloc_context();

    int open_result_code = avformat_open_input(&format_context, input_p_s, NULL, NULL);
    if (open_result_code != 0) {
        LOGE("%s", "打开输入视频文件失败");
        return;
    }
    //获取视频信息
    if (avformat_find_stream_info(format_context, NULL) < 0) {
        LOGE("%s", "获取视频信息失败");
        return;
    }
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
    //5，打开解码器
    int open_codec_result_code = avcodec_open2(codecContext, pCodec, NULL);
    if (open_codec_result_code < 0) {
        LOGE("%s", "找不到解码或者解码器无法打开");
        return;
    }
    player->input_codec_context[stream_index] = codecContext;

}


void decode_video(Player *player, AVPacket *avPacket) {
    AVCodecContext *codecContext = player->input_codec_context[player->video_stream_index];
    int len, got_frame, framecount = 0;
    //像素数据（解码数据）
    AVFrame *avFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();

    //绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;

    len = avcodec_decode_video2(codecContext, avFrame, &got_frame, avPacket);
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
        //unlock 解锁
        //绘制完成
        ANativeWindow_unlockAndPost(player->nativeWindow);
        usleep(1000 * 16);
    }

    av_frame_free(&avFrame);
    av_frame_free(&rgbFrame);


}

void *decode_data(void *arg) {
    struct Player *player = (struct Player *) (arg);
    //6. 一帧一帧读取压缩的数据AVPacket
    //这里为啥要开辟啊
    //压缩数据
    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFormatContext *formatContext = player->input_format_context;
    int video_frame_count = 0;

    //不断的读取压缩数据
    while (av_read_frame(formatContext, avPacket) >= 0) {
        if (avPacket->stream_index == player->audio_stream_index) {
            LOGE("多少针%d", video_frame_count++);
            decode_audio(player, avPacket);
        }
//        if (avPacket->stream_index == player->video_stream_index) {
////            decode_video(player, avPacket);
//
//        } else if (avPacket->stream_index == player->audio_stream_index) {
//            decode_audio(player, avPacket);
//        }
        av_packet_unref(avPacket);
        LOGE("%s", "解引用");
    }
    return NULL;
}

void decode_video_prepare(JNIEnv *env, struct Player *player, jobject surface) {
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
void jni_audio_init(JNIEnv *env, jobject jthiz, struct Player *player) {
    //JNI begin------------------
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

//解码在子线程里面
void decode_audio(struct Player *player, AVPacket *avPacket) {

    AVCodecContext *codec_ctx = player->input_codec_context[player->audio_stream_index];
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //16bit 44100 PCM 数据(重采样缓冲区)
    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);
    int got_frame = 0, index = 0, ret;
    ret = decode(codec_ctx, frame, &got_frame, avPacket);
    LOGE("%d------%d", ret,got_frame);
    if (ret >= 0) {
        LOGE("%s", "解码完成");
    } else {
        LOGE("%s", "解码失败");
    }
    if (got_frame > 0) {
        LOGI("解码got_frame：%d", index++);
//        swr_convert(player->swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE,
//                    (const uint8_t **) frame->data, frame->nb_samples);
//        //获取sample的size
//        int out_buffer_size = av_samples_get_buffer_size(NULL, player->out_channel_nb,
//                                                         frame->nb_samples, player->out_sample_fmt,
//                                                         1);
//        // 斜道一个pcm文件去
//        // fwrite(out_buffer, 1, out_buffer_size, fp_pcm);
//        JavaVM *javaVM = player->javaVM;
//        JNIEnv *env;
//        javaVM->AttachCurrentThread(&env, NULL);
//        //out_buffer缓冲区数据，转成byte数组
//        jbyteArray audio_sample_array = env->NewByteArray(out_buffer_size);
//        jbyte *sample_bytep = env->GetByteArrayElements(audio_sample_array, NULL);
//        //out_buffer的数据复制到sampe_bytep
//        memcpy(sample_bytep, out_buffer, out_buffer_size);
//        env->ReleaseByteArrayElements(audio_sample_array, sample_bytep, 0);
//
//        //AudioTrack.write PCM数据
//        env->CallIntMethod(player->audio_track, player->audio_track_write_mid,
//                           audio_sample_array, 0, out_buffer_size);
//        env->DeleteLocalRef(audio_sample_array);
////        env->DeleteGlobalRef(player->audio_track);
//        javaVM->DetachCurrentThread();
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

void JNICALL
Java_com_example_administrator_ffmpeg_1master_VideoUtil_play(JNIEnv *env, jobject instance,
                                                             jstring input_, jobject surface) {
    const char *input_p = env->GetStringUTFChars(input_, NULL);
    struct Player *player = (struct Player *) (malloc(sizeof(struct Player)));
    player->javaVM = (JavaVM *) (env->GetJavaVM(&(player->javaVM)));
    //进行初始化
    init_input_from_context(player, input_p);

    int video_stream_index_i = player->video_stream_index;
    int audio_stream_index_i = player->audio_stream_index;
    init_codec_context(player, video_stream_index_i);
    init_codec_context(player, audio_stream_index_i);
    decode_video_prepare(env, player, surface);
    decode_audio_prepare(env, player);

    jni_audio_init(env, instance, player);




//    ANativeWindow_release(player->nativeWindow);
    //解码视频
//    pthread_create(&(player->decode_threads[video_stream_index_i]), NULL, decode_data,
//                   (void *) player);
    pthread_create(&(player->decode_threads[audio_stream_index_i]), NULL, decode_data,
                   (void *) player);
//    ANativeWindow_release(nativeWindow);
//    av_frame_free(&avFrame);
//    avformat_free_context(player->input_format_context);
//    env->ReleaseStringUTFChars(input_, input_p);
//
//    free(player);

}