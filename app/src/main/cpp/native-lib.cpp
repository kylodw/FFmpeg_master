
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
    sprintf(input_str, "%s", env->GetStringUTFChars(input_jstr, NULL));
    sprintf(output_str, "%s", env->GetStringUTFChars(output_jstr, NULL));

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


    sprintf(info, "[Input     ]%s\n", input_str);
    sprintf(info, "%s[Output    ]%s\n", info, output_str);
    sprintf(info, "%s[Format    ]%s\n", info, pFormatCtx->iformat->name);
    sprintf(info, "%s[Codec     ]%s\n", info, pCodecCtx->codec->name);
    sprintf(info, "%s[Resolution]%dx%d\n", info, pCodecCtx->width, pCodecCtx->height);


    fp_yuv = fopen(output_str, "wb+");
    if (fp_yuv == NULL) {
        printf("Cannot open output file.\n");
        return -1;
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

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_administrator_ffmpeg_1master_MainActivity_stream(JNIEnv *env, jobject instance,
                                                                  jstring input_jstr,
                                                                  jstring output_jstr) {
//    AVOutputFormat *ofmt = NULL;
//    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
//    AVPacket pkt;
//    int videoindex = -1;
////    int ret, i;
//    char input_str[500] = {0};
//    char output_str[500] = {0};
////    char info[1000] = {0};
//    sprintf(input_str, "%s", env->GetStringUTFChars(input_jstr, NULL));
//    sprintf(output_str, "%s", env->GetStringUTFChars(output_jstr, NULL));
//
//    //input_str  = "cuc_ieschool.flv";
//    //output_str = "rtmp://localhost/publishlive/livestream";
//    //output_str = "rtp://233.233.233.233:6666";
//
//    //FFmpeg av_log() callback
//    av_log_set_callback(custom_log);
//
//    av_register_all();
//    //Network
//    avformat_network_init();
//
//    AVFormatContext *ictx = NULL;
//
//    AVOutputFormat *ofmt = NULL;
//    char *inUrl = const_cast<char *>(env->GetStringUTFChars(input_jstr, NULL));
//    char *outUrl = const_cast<char *>(env->GetStringUTFChars(input_jstr, NULL));
//    //打开文件，解封文件头
//    int ret = avformat_open_input(&ictx, inUrl, 0, NULL);
//    if (ret < 0) {
//    }
//    //获取音频视频的信息 .h264 flv 没有头信息
//    ret = avformat_find_stream_info(ictx, 0);
//    if (ret != 0) {
//    }
//    //打印视频视频信息
//    //0打印所有  inUrl 打印时候显示，
//    av_dump_format(ictx, 0, inUrl, 0);
//
//    //////////////////////////////////////////////////////////////////
//    //                   输出流处理部分
//    /////////////////////////////////////////////////////////////////
//    AVFormatContext *octx = NULL;
//    //如果是输入文件 flv可以不传，可以从文件中判断。如果是流则必须传
//    //创建输出上下文
//    ret = avformat_alloc_output_context2(&octx, NULL, "flv", outUrl);
//    if (ret < 0) {
//    }
//    ofmt = octx->oformat;
//    int i;
//    //for (i = 0; i < ictx->nb_streams; i++) {
//    //  cout << "i " << i <<"  "<< ictx->nb_streams<< endl;
//    //  AVStream *in_stream = ictx->streams[i];
//    //  AVCodec *codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
//    //  AVStream *out_stream = avformat_new_stream(octx, codec);
//    //  if (!out_stream) {
//    //      printf("Failed allocating output stream\n");
//    //      ret = AVERROR_UNKNOWN;
//    //  }
//    //  AVCodecContext *pCodecCtx = avcodec_alloc_context3(codec);
//    //  ret = avcodec_parameters_to_context(pCodecCtx, in_stream->codecpar);
//    //  if (ret < 0) {
//    //      printf("Failed to copy context input to output stream codec context\n");
//    //  }
//    //  pCodecCtx->codec_tag = 0;
//    //  if (octx->oformat->flags & AVFMT_GLOBALHEADER) {
//    //      pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
//    //  }
//    //  ret = avcodec_parameters_from_context(out_stream->codecpar, pCodecCtx);
//    //  if (ret < 0) {
//    //      printf("Failed to copy context input to output stream codec context\n");
//    //  }
//    //}
//
//    for (i = 0; i < ictx->nb_streams; i++) {
//
//        //获取输入视频流
//        AVStream *in_stream = ictx->streams[i];
//        //为输出上下文添加音视频流（初始化一个音视频流容器）
//        AVStream *out_stream = avformat_new_stream(octx, in_stream->codec->codec);
//        if (!out_stream) {
//            printf("未能成功添加音视频流\n");
//            ret = AVERROR_UNKNOWN;
//        }
//
//        //将输入编解码器上下文信息 copy 给输出编解码器上下文
//        //ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
//        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
//        //ret = avcodec_parameters_from_context(out_stream->codecpar, in_stream->codec);
//        //ret = avcodec_parameters_to_context(out_stream->codec, in_stream->codecpar);
//        if (ret < 0) {
//            printf("copy 编解码器上下文失败\n");
//        }
//        out_stream->codecpar->codec_tag = 0;
//
//        out_stream->codec->codec_tag = 0;
//        if (octx->oformat->flags & AVFMT_GLOBALHEADER) {
//            out_stream->codec->flags = out_stream->codec->flags | CODEC_FLAG_GLOBAL_HEADER;
//        }
//    }
//
//    //输入流数据的数量循环
//    for (i = 0; i < ictx->nb_streams; i++) {
//        if (ictx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//            videoindex = i;
//            break;
//        }
//    }
//
//    av_dump_format(octx, 0, outUrl, 1);
//
//    //////////////////////////////////////////////////////////////////
//    //                   准备推流
//    /////////////////////////////////////////////////////////////////
//
//    //打开IO
//    ret = avio_open(&octx->pb, outUrl, AVIO_FLAG_WRITE);
//    if (ret < 0) {
//    }
//
//    //写入头部信息
//    ret = avformat_write_header(octx, 0);
//    if (ret < 0) {
//    }
//    //推流每一帧数据
//    //int64_t pts  [ pts*(num/den)  第几秒显示]
//    //int64_t dts  解码时间 [P帧(相对于上一帧的变化) I帧(关键帧，完整的数据) B帧(上一帧和下一帧的变化)]  有了B帧压缩率更高。
//    //uint8_t *data
//    //int size
//    //int stream_index
//    //int flag
//    AVPacket pkt;
//    //获取当前的时间戳  微妙
//    long long start_time = av_gettime();
//    long long frame_index = 0;
//    while (1) {
//        //输入输出视频流
//        AVStream *in_stream, *out_stream;
//        //获取解码前数据
//        ret = av_read_frame(ictx, &pkt);
//        if (ret < 0) {
//            break;
//        }
//
//        /*
//        PTS（Presentation Time Stamp）显示播放时间
//        DTS（Decoding Time Stamp）解码时间
//        */
//        //没有显示时间（比如未解码的 H.264 ）
//        if (pkt.pts == AV_NOPTS_VALUE) {
//            //AVRational time_base：时基。通过该值可以把PTS，DTS转化为真正的时间。
//            AVRational time_base1 = ictx->streams[videoindex]->time_base;
//
//            //计算两帧之间的时间
//            /*
//            r_frame_rate 基流帧速率  （不是太懂）
//            av_q2d 转化为double类型
//            */
//            int64_t calc_duration =
//                    (double) AV_TIME_BASE / av_q2d(ictx->streams[videoindex]->r_frame_rate);
//
//            //配置参数
//            pkt.pts = (double) (frame_index * calc_duration) /
//                      (double) (av_q2d(time_base1) * AV_TIME_BASE);
//            pkt.dts = pkt.pts;
//            pkt.duration = (double) calc_duration / (double) (av_q2d(time_base1) * AV_TIME_BASE);
//        }
//
//        //延时
//        if (pkt.stream_index == videoindex) {
//            AVRational time_base = ictx->streams[videoindex]->time_base;
//            AVRational time_base_q = {1, AV_TIME_BASE};
//            //计算视频播放时间
//            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
//            //计算实际视频的播放时间
//            int64_t now_time = av_gettime() - start_time;
//
//            AVRational avr = ictx->streams[videoindex]->time_base;
//            if (pts_time > now_time) {
//                //睡眠一段时间（目的是让当前视频记录的播放时间与实际时间同步）
//                av_usleep((unsigned int) (pts_time - now_time));
//            }
//        }
//
//        in_stream = ictx->streams[pkt.stream_index];
//        out_stream = octx->streams[pkt.stream_index];
//
//        //计算延时后，重新指定时间戳
//        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
//                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
//                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
//        pkt.duration = (int) av_rescale_q(pkt.duration, in_stream->time_base,
//                                          out_stream->time_base);
//        //字节流的位置，-1 表示不知道字节流位置
//        pkt.pos = -1;
//
//        if (pkt.stream_index == videoindex) {
//            frame_index++;
//        }
//
//        //向输出上下文发送（向地址推送）
//        ret = av_interleaved_write_frame(octx, &pkt);
//
//        if (ret < 0) {
//            printf("发送数据包出错\n");
//            break;
//        }
//
//        //释放
//        av_packet_unref(&pkt);
//    }
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