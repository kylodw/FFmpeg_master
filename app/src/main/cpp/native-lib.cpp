
#include <unistd.h>
#include "common.h"


extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libavutil/mathematics.h"


JNIEXPORT jstring JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */);

JNIEXPORT jstring JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_impleStringFromJNI(JNIEnv *env,
                                                                              jobject instance);


JNIEXPORT jint JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_decode(JNIEnv *env, jobject instance,
                                                                  jstring input_jstr,
                                                                  jstring output_jstr);


int ffmpegmain(int argc, char **argv);

JNIEXPORT jint JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_ffmpegcore(JNIEnv *env, jobject instance,
                                                                      jobjectArray argv);

}
//c  end


JNIEXPORT jstring JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


JNIEXPORT jstring JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_impleStringFromJNI(JNIEnv *env,
                                                                              jobject instance) {
    char info[10000] = {0};
    sprintf(info, "%s\n", avcodec_configuration());
    return env->NewStringUTF(info);
}

void custom_log(void *ptr, int level, const char *fmt, va_list vl) {
    FILE *fp = fopen("/storage/emulated/0/av_log.txt", "a+");
    if (fp) {
        vfprintf(fp, fmt, vl);
        fflush(fp);
        fclose(fp);
    }
}

JNIEXPORT jint JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_decode(JNIEnv *env, jobject instance,
                                                                  jstring input_jstr,
                                                                  jstring output_jstr) {
    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    uint8_t *out_buffer;
    AVPacket *packet;
    int y_size;
    int ret, got_picture;
    struct SwsContext *img_convert_ctx;
    FILE *fp_yuv;
    int frame_cnt;
    clock_t time_start, time_finish;
    double time_duration = 0.0;

    char input_str[500] = {0};
    char output_str[500] = {0};
    char info[1000] = {0};

    //FFmpeg av_log() callback
    av_log_set_callback(custom_log);
    //注册所有组件
    av_register_all();
    //也可以单个注册

    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    //打开视频文件
    if (avformat_open_input(&pFormatCtx, input_str, NULL, NULL) != 0) {
        LOGE("Couldn't open input stream.\n");
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("Couldn't find stream information.\n");
        return -1;
    }
    videoindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    if (videoindex == -1) {
        LOGE("Couldn't find a video stream.\n");
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    //找到解码器
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGE("找不到解码器\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("不能打开解码器\n");
        return -1;
    } else {
        LOGE("打开解码器");
    }

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    out_buffer = (unsigned char *) av_malloc(
            av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    //准备读取
    //AVPacket用于存储一帧一帧的压缩数据（H264）
    //缓冲区 开辟空间
    packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                     pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC, NULL, NULL, NULL);
    fp_yuv = fopen(output_str, "wb+");
    if (fp_yuv == NULL) {
        printf("Cannot open output file.\n");
        return -1;
    } else {
        LOGE("打开文件");
    }

    frame_cnt = 0;
    time_start = clock();

    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == videoindex) {
            //解码
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0) {
                LOGE("Decode Error.\n");
                return -1;
            } else {
                LOGE("decode");
            }
            if (got_picture) {
                sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize,
                          0, pCodecCtx->height,
                          pFrameYUV->data, pFrameYUV->linesize);

                y_size = pCodecCtx->width * pCodecCtx->height;
                fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
                fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
                fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
                //Output info
                char pictype_str[10] = {0};
                switch (pFrame->pict_type) {
                    case AV_PICTURE_TYPE_I:
                        sprintf(pictype_str, "I");
                        break;
                    case AV_PICTURE_TYPE_P:
                        sprintf(pictype_str, "P");
                        break;
                    case AV_PICTURE_TYPE_B:
                        sprintf(pictype_str, "B");
                        break;
                    default:
                        sprintf(pictype_str, "Other");
                        break;
                }
                LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
                frame_cnt++;
            }
        }
        av_packet_unref(packet);
    }
    //flush decoder
    //FIX: Flush Frames remained in Codec
    while (1) {
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if (ret < 0)
            break;
        if (!got_picture)
            break;
        sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
                  pCodecCtx->height,
                  pFrameYUV->data, pFrameYUV->linesize);
        int y_size = pCodecCtx->width * pCodecCtx->height;
        fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y
        fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
        fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
        //Output info
        char pictype_str[10] = {0};
        switch (pFrame->pict_type) {
            case AV_PICTURE_TYPE_I:
                sprintf(pictype_str, "I");
                break;
            case AV_PICTURE_TYPE_P:
                sprintf(pictype_str, "P");
                break;
            case AV_PICTURE_TYPE_B:
                sprintf(pictype_str, "B");
                break;
            default:
                sprintf(pictype_str, "Other");
                break;
        }
        LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
        frame_cnt++;
    }
    time_finish = clock();
    time_duration = (double) (time_finish - time_start);

    sprintf(info, "%s[Time      ]%fms\n", info, time_duration);
    sprintf(info, "%s[Count     ]%d\n", info, frame_cnt);

    sws_freeContext(img_convert_ctx);

    fclose(fp_yuv);

    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;

}
/**
 * 推流
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_stream(JNIEnv *env, jobject instance,
                                                                  jstring input_jstr,
                                                                  jstring output_jstr) {
    AVOutputFormat *outputFormat = NULL;
    AVPacket pkt;
    const char *in_file_name, *out_file_name;
    int frame_index = 0;
    int64_t start_time = av_gettime();
    int i = 0, videoIndex = 0;
    int find_stream;
    in_file_name = env->GetStringUTFChars(input_jstr, NULL);
    out_file_name = env->GetStringUTFChars(output_jstr, NULL);
    av_register_all();//注册器
    AVFormatContext *input_f_cxt = avformat_alloc_context(), *output_f_cxt = avformat_alloc_context();
    avformat_network_init();//网络

    int ret = avformat_open_input(&input_f_cxt, in_file_name, NULL, NULL);
    if (ret < 0) {
        LOGE("%s", "打开文件失败!");
        goto end;
    }
     find_stream = avformat_find_stream_info(input_f_cxt, 0);
    if (find_stream < 0) {
        LOGE("%s", "输入流错误");
        goto end;
    }



    for (; i < input_f_cxt->nb_streams; i++) {
        if (input_f_cxt->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
        }
    }
    av_dump_format(input_f_cxt, 0, in_file_name, 0);

    avformat_alloc_output_context2(&output_f_cxt, NULL, "flv", out_file_name);

    if (!output_f_cxt) {
        LOGE("%s", "创建output_context失败");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    outputFormat = output_f_cxt->oformat;
    i = 0;
    for (; i < input_f_cxt->nb_streams; i++) {
        AVStream *in_stream = input_f_cxt->streams[i];
        AVStream *out_stream = avformat_new_stream(output_f_cxt, in_stream->codec->codec);
        if (!out_stream) {
            LOGE("%s", "没有输出流");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        if (ret < 0) {
            LOGE("%s", "拷贝失败");
            goto end;
        }
        out_stream->codec->codec_tag = 0;
        if (output_f_cxt->oformat->flags & AVFMT_GLOBALHEADER) {
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    av_dump_format(output_f_cxt, 0, out_file_name, 1);

    if (!(outputFormat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&output_f_cxt->pb, out_file_name, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("%s", "不能打开URL");
            goto end;
        }
    }

    ret = avformat_write_header(output_f_cxt, NULL);
    if (ret < 0) {
        LOGE("%s:%d", "写入头失败", ret);
        goto end;
    }

    while (1) {
        AVStream *in_stream, *out_stream;
        //裸流
        ret = av_read_frame(input_f_cxt, &pkt);
        if (ret < 0) {
            break;
        }
        if (pkt.pts == AV_NOPTS_VALUE) {
            AVRational time_base = input_f_cxt->streams[videoIndex]->time_base;

            int64_t calc_dur = static_cast<int64_t>(AV_TIME_BASE /
                                                    av_q2d(input_f_cxt->streams[videoIndex]->r_frame_rate));
            pkt.pts = static_cast<int64_t>((double) (frame_index * calc_dur) /
                                           (av_q2d(time_base) * AV_TIME_BASE));
            pkt.dts = pkt.pts;
            pkt.duration = static_cast<int64_t>((double) calc_dur /
                                                (double) (av_q2d(time_base) * AV_TIME_BASE));
        }

        if (pkt.stream_index == videoIndex) {
            AVRational time_base = input_f_cxt->streams[videoIndex]->time_base;
            AVRational time_base_q = {1, AV_TIME_BASE};

            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time) {
                av_usleep(static_cast<unsigned int>(pts_time - now_time));
            }
        }
        in_stream = input_f_cxt->streams[pkt.stream_index];
        out_stream = output_f_cxt->streams[pkt.stream_index];

        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        if (pkt.stream_index == videoIndex) {
            LOGE("发送视频帧: %8d", frame_index);
            frame_index++;
        }
        //写入output_cxt
        ret = av_interleaved_write_frame(output_f_cxt, &pkt);
        if (ret < 0) {
            LOGE("%s", "写入帧失败");
            goto end;
        }
        av_packet_unref(&pkt);

    }
    av_write_trailer(output_f_cxt);
    end:
    avformat_close_input(&input_f_cxt);
    if (output_f_cxt && !(outputFormat->flags && AVFMT_NOFILE)) {
        avio_close(output_f_cxt->pb);
    }
    avformat_free_context(output_f_cxt);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

JNIEXPORT jint JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_ffmpegcore(JNIEnv *env, jobject instance,
                                                                      jobjectArray argv) {

    // TODO
    av_log_set_callback(custom_log);

    jint margc = env->GetArrayLength(argv);
    char **margv = static_cast<char **>(malloc(sizeof(char *) * margc));
    int i = 0;
    for (; i < margc; i++) {
        jstring j_str = static_cast<jstring>(env->GetObjectArrayElement(argv, i));
        const char *tmp = env->GetStringUTFChars(j_str, NULL);
        margv[i] = static_cast<char *>(malloc(sizeof(char) * 1024));
        strcpy(margv[i], tmp);
    }

    ffmpegmain(margc, margv);
    for (int i = 0; i < margc; i++) {
        free(margv[i]);
    }
    free(margv);
    return 0;

}