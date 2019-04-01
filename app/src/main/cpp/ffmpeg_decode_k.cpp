//
// Created by kylodw on 2019/3/30.
//
#include "common.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <unistd.h>

//按照c的编译方式统一编译（混编）
extern "C" {
//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
#include "libyuv.h"
#include "libyuv/convert_argb.h"
#include "libswresample/swresample.h"

int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt);
}
using namespace libyuv;
#define MAX_AUDIO_FRME_SIZE 48000 * 4


JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_VideoUtil_newdecode(JNIEnv *env, jclass type,
                                                                  jstring input_, jstring output_) {
    const char *input_p = env->GetStringUTFChars(input_, NULL);
    const char *out_p = env->GetStringUTFChars(output_, NULL);
    av_register_all();
    AVFormatContext *formatContext = avformat_alloc_context();
    int open_result_code = avformat_open_input(&formatContext, input_p, NULL, NULL);
    if (open_result_code != 0) {
        LOGE("%s", "打开输入视频文件失败");
        return;
    }
    //获取视频信息
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGE("%s", "获取视频信息失败");
        return;
    }
    //解码器  音频和视频是分开的。
//    AVStream //视频流
    //解完封装  根据索引位置去解码
    //找到视频AVStream的索引位置
    int i = 0;
    int video_stream_index = -1;
    //多少个视频数据
    for (; i < formatContext->nb_streams; i++) {
        //根据类型判断，是否是视频流
        //如果需要字幕 也需要一个AVStream流
        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        } else {
            LOGE("%s", "没有找到视频流");
        }
    }
    //4，解码器  解码上下文
    //AVCodecContext 保存了编解码的相关信息
    AVCodecContext *codecContext = formatContext->streams[video_stream_index]->codec;
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
    //6. 一帧一帧读取压缩的数据AVPacket
    //这里为啥要开辟啊
    //压缩数据
    AVPacket *avPacket;
    av_init_packet(avPacket);
    //像素数据（解码数据）
    AVFrame *avFrame = av_frame_alloc();
    AVFrame *yuvFrame = av_frame_alloc();
    //只有指定了AVFrame的像素格式，画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buff = (uint8_t *) (av_malloc(
            avpicture_get_size(AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height)));
    avpicture_fill((AVPicture *) (yuvFrame), out_buff, AV_PIX_FMT_YUV420P, codecContext->width,
                   codecContext->height);

    //输出文件
    FILE *fp_yuv = fopen(out_p, "wb");
    struct SwsContext *sws_ctx = sws_getContext(
            codecContext->width, codecContext->height, codecContext->pix_fmt,
            codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, NULL, NULL, NULL);

    int len, got_frame, framecount = 0;
    //
    while (av_read_frame(formatContext, avPacket) >= 0) {
        len = avcodec_decode_video2(codecContext, avFrame, &got_frame, avPacket);
        //非零，正在解码
        if (got_frame) {
            //frame->yuvFrame(YUV420P)
            sws_scale(sws_ctx,
                      avFrame->data, avFrame->linesize, 0, avFrame->height,
                      yuvFrame->data, yuvFrame->linesize);

            //想YUV文件保存解码之后的帧数据
            //AVFrame->YUV
            //一个像素包含一个Y
            int y_size = codecContext->width * codecContext->height;
            fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);
            fwrite(yuvFrame->data[1], 1, y_size / 4, fp_yuv);
            fwrite(yuvFrame->data[2], 1, y_size / 4, fp_yuv);

            LOGE("解码%d帧", framecount++);
        }
        av_packet_unref(avPacket);
    }
    fclose(fp_yuv);
    av_frame_free(&avFrame);
    avcodec_close(codecContext);
    avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(input_, input_p);
    env->ReleaseStringUTFChars(output_, out_p);

}

extern "C" JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_VideoUtil_render(JNIEnv *env, jobject instance,
                                                               jstring input_, jobject surface) {
    const char *input_p = env->GetStringUTFChars(input_, NULL);
    av_register_all();

    AVFormatContext *formatContext = avformat_alloc_context();

    int open_result_code = avformat_open_input(&formatContext, input_p, NULL, NULL);
    if (open_result_code != 0) {
        LOGE("%s", "打开输入视频文件失败");
        return;
    }
    //获取视频信息
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGE("%s", "获取视频信息失败");
        return;
    }
    //解码器  音频和视频是分开的。
//    AVStream //视频流
    //解完封装  根据索引位置去解码
    //找到视频AVStream的索引位置
    int i = 0;
    int video_stream_index = -1;
    //多少个视频数据
    for (; i < formatContext->nb_streams; i++) {
        //根据类型判断，是否是视频流
        //如果需要字幕 也需要一个AVStream流
        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        } else {
            LOGE("%s", "没有找到视频流");
        }
    }
    //4，解码器  解码上下文
    //AVCodecContext 保存了编解码的相关信息
    AVCodecContext *codecContext = formatContext->streams[video_stream_index]->codec;
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
    //6. 一帧一帧读取压缩的数据AVPacket
    //这里为啥要开辟啊
    //压缩数据
    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    //像素数据（解码数据）
    AVFrame *avFrame = av_frame_alloc();
    AVFrame *rgbFrame = av_frame_alloc();

    //native 绘制
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    //绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;


    int len, got_frame, framecount = 0;
    //
    while (av_read_frame(formatContext, avPacket) >= 0) {
        len = avcodec_decode_video2(codecContext, avFrame, &got_frame, avPacket);
        //非零，正在解码
        if (got_frame) {
            LOGE("解码%d帧", framecount++);

            //AudioTrack.writePCM数据

            //lock
            //设置缓冲区的属性 宽高像素格式
            //最终缓冲区的数据  surface_view
            ANativeWindow_setBuffersGeometry(nativeWindow, codecContext->width,
                                             codecContext->height, WINDOW_FORMAT_RGBA_8888);
            ANativeWindow_lock(nativeWindow, &outBuffer, NULL);
            //fix buffer  要转换成RGBA_8888  YUV
            LOGE("输出的格式%c", codecContext->pix_fmt);

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
            ANativeWindow_unlockAndPost(nativeWindow);
        }
        av_packet_unref(avPacket);
    }
    ANativeWindow_release(nativeWindow);
    av_frame_free(&avFrame);
    avcodec_close(codecContext);
    avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(input_, input_p);

}


JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_VideoUtil_sound(JNIEnv *env, jobject jthiz,
                                                              jstring input_jstr,
                                                              jstring output_jstr) {
    const char *input_cstr = env->GetStringUTFChars(input_jstr, NULL);
    const char *output_cstr = env->GetStringUTFChars(output_jstr, NULL);
    LOGI("%s", "sound");
    //注册组件
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //打开音频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOGI("%s", "无法打开音频文件");
        return;
    }
    //获取输入文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGI("%s", "无法获取输入文件信息");
        return;
    }
    //获取音频流索引位置
    int i = 0, audio_stream_idx = -1;
    for (; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
            break;
        }
    }

    //获取解码器
    AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
    if (codec == NULL) {
        LOGI("%s", "无法获取解码器");
        return;
    }
    //打开解码器
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        LOGI("%s", "无法打开解码器");
        return;
    }
    //压缩数据
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrCtx = swr_alloc();

    //重采样设置参数-------------start
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

    //重采样设置参数-------------end

    //JNI begin------------------
    //JasonPlayer
    jclass player_class = env->GetObjectClass(jthiz);

    //AudioTrack对象
    jmethodID create_audio_track_mid = env->GetMethodID(player_class, "createAudioTrack",
                                                        "(II)Landroid/media/AudioTrack;");
    jobject audio_track = env->CallObjectMethod(jthiz, create_audio_track_mid, out_sample_rate,
                                                out_channel_nb);

    //调用AudioTrack.play方法
    jclass audio_track_class = env->GetObjectClass(audio_track);

    jmethodID audio_track_play_mid = env->GetMethodID(audio_track_class, "play", "()V");
    env->CallVoidMethod(audio_track, audio_track_play_mid);

    //AudioTrack.write
    jmethodID audio_track_write_mid = env->GetMethodID(audio_track_class, "write", "([BII)I");

    //JNI end------------------
    FILE *fp_pcm = fopen(output_cstr, "wb");

    //16bit 44100 PCM 数据
    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);

    int got_frame = 0, index = 0, ret;
    //不断读取压缩数据
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //解码音频类型的Packet
        if (packet->stream_index == audio_stream_idx) {
            ret = decode(codecCtx, frame, &got_frame, packet);
            LOGI("解码的ret%d", ret);
            if (ret >= 0) {
                LOGI("%s", "解码完成");
            }
            //解码一帧成功
            if (got_frame > 0) {
                LOGI("解码：%d", index++);
                swr_convert(swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE,
                            (const uint8_t **) frame->data, frame->nb_samples);
                //获取sample的size
                int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                                 frame->nb_samples, out_sample_fmt,
                                                                 1);
                fwrite(out_buffer, 1, out_buffer_size, fp_pcm);

                //out_buffer缓冲区数据，转成byte数组
                jbyteArray audio_sample_array = env->NewByteArray(out_buffer_size);
                jbyte *sample_bytep = env->GetByteArrayElements(audio_sample_array, NULL);
                //out_buffer的数据复制到sampe_bytep
                memcpy(sample_bytep, out_buffer, out_buffer_size);
                //同步
                env->ReleaseByteArrayElements(audio_sample_array, sample_bytep, 0);

                //AudioTrack.write PCM数据
                env->CallIntMethod(audio_track, audio_track_write_mid,
                                   audio_sample_array, 0, out_buffer_size);
                //释放局部引用
                env->DeleteLocalRef(audio_sample_array);
                usleep(1000 * 16);
            }
        }

        av_packet_unref(packet);
    }

    av_frame_free(&frame);
    av_free(out_buffer);

    swr_free(&swrCtx);
    avcodec_close(codecCtx);
    avformat_close_input(&pFormatCtx);

    env->ReleaseStringUTFChars(input_jstr, input_cstr);
    env->ReleaseStringUTFChars(output_jstr, output_cstr);

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