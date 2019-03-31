//
// Created by kylodw on 2019/3/30.
//
#include "common.h"


extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>

}


JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_VideoUtil_newdecode(JNIEnv *env, jclass type,
                                                                  jstring input_, jstring output_){
    const char* input_p=env->GetStringUTFChars(input_,NULL);
    const  char* out_p=env->GetStringUTFChars(output_,NULL);
    av_register_all();
    AVFormatContext * formatContext=avformat_alloc_context();
    int open_result_code=avformat_open_input(&formatContext,input_p,NULL,NULL);
    if(open_result_code!=0){
        LOGE("%s","打开输入视频文件失败");
        return;
    }
    //获取视频信息
    if (avformat_find_stream_info(formatContext,NULL)<0){
        LOGE("%s","获取视频信息失败");
        return;
    }
    //解码器  音频和视频是分开的。
//    AVStream //视频流
    //解完封装  根据索引位置去解码
    //找到视频AVStream的索引位置
    int i=0;
    int video_stream_index=-1;
    //多少个视频数据
    for (;i<formatContext->nb_streams;i++){
        //根据类型判断，是否是视频流
        //如果需要字幕 也需要一个AVStream流
        if(formatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
            video_stream_index=i;
            break;
        } else{
            LOGE("%s","没有找到视频流");
        }
    }
    //4，解码器  解码上下文
    //AVCodecContext 保存了编解码的相关信息
    AVCodecContext *codecContext=formatContext->streams[video_stream_index]->codec;
    //根据id找到对应的解码器
    AVCodec *pCodec=avcodec_find_decoder(codecContext->codec_id);
    if(pCodec==NULL){
        LOGE("%s","无法解码");
        return;
    }
    //5，打开解码器
   int open_codec_result_code= avcodec_open2(codecContext,pCodec,NULL);
    if(open_codec_result_code<0){
        LOGE("%s","找不到解码或者解码器无法打开");
        return;
    }
    //6. 一帧一帧读取压缩的数据AVPacket
    //这里为啥要开辟啊
    //压缩数据
    AVPacket *avPacket;
    av_init_packet(avPacket);
    //像素数据（解码数据）
    AVFrame *avFrame=av_frame_alloc();
    AVFrame *yuvFrame=av_frame_alloc();
    //只有指定了AVFrame的像素格式，画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t  *out_buff= (uint8_t *)(av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height)));
    avpicture_fill((AVPicture *)(yuvFrame), out_buff, AV_PIX_FMT_YUV420P, codecContext->width, codecContext->height);

    //输出文件
    FILE* fp_yuv=fopen(out_p,"wb");
    struct  SwsContext *sws_ctx=sws_getContext(
            codecContext->width,codecContext->height,codecContext->pix_fmt,
            codecContext->width,codecContext->height,AV_PIX_FMT_YUV420P,
            SWS_BILINEAR,NULL,NULL,NULL);

    int len,got_frame,framecount=0;
    //
    while (av_read_frame(formatContext,avPacket)>=0){
        len=avcodec_decode_video2(codecContext,avFrame,&got_frame,avPacket);
        //非零，正在解码
        if(got_frame){
            //frame->yuvFrame(YUV420P)
            sws_scale(sws_ctx,
                    avFrame->data,avFrame->linesize,0,avFrame->height,
                    yuvFrame->data,yuvFrame->linesize);

            //想YUV文件保存解码之后的帧数据
            //AVFrame->YUV
            //一个像素包含一个Y
            int y_size=codecContext->width*codecContext->height;
            fwrite(yuvFrame->data[0],1,y_size,fp_yuv);
            fwrite(yuvFrame->data[1],1,y_size/4,fp_yuv);
            fwrite(yuvFrame->data[2],1,y_size/4,fp_yuv);

            LOGE("解码%d帧",formatContext++);
        }
        av_packet_unref(avPacket);
    }
    fclose(fp_yuv);
    av_frame_free(&avFrame);
    avcodec_close(codecContext);
    avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(input_,input_p);
    env->ReleaseStringUTFChars(output_,out_p);

}

