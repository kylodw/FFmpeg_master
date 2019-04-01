//
// Created by kylodw on 2019/4/1.
//

#ifndef FFMPEG_MASTER_VIDEO_PLAY_H
#define FFMPEG_MASTER_VIDEO_PLAY_H


#include <android/native_window.h>
#include <pthread.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <malloc.h>
#include <cstring>
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libyuv.h>
};
//nb_streams  音频流，视频流，字幕
#define MAX_STREAM 2
#define MAX_AUDIO_FRME_SIZE 48000 * 4
struct _Player {

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

    pthread_t  thread_read_from_stream;
    //流的总个数
    int capture_stream_no;
    //音频视频队列数组
    Queue *packets[MAX_STREAM];
};
typedef struct _Player Player;
/**
 * 解码数据
 */
struct  _DecodeData{
    Player *player;
    int stream_index;
};
typedef  struct _DecodeData DecodeData;


extern "C" {

void init_codec_context(Player *player, int stream_index);

void init_input_from_context(Player *player, const char *input_p_s);

void *decode_data(void *arg);

void decode_video_prepare(JNIEnv *env, Player *player, jobject surface);
void decode_video(Player *player, AVPacket *pPacket);
void decode_audio(Player *player, AVPacket *avPacket);
void *player_read_from_stream(void *arg);

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_VideoUtil_play(JNIEnv *env, jobject instance,
                                                             jstring input_, jobject surface);
void decode_audio_prepare(JNIEnv *pEnv, Player *pPlayer);

int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt);
void jni_audio_init(JNIEnv *env, jobject jthiz, Player *player);
void player_alloc_queues();
void* packet_free_func(AVPacket *packet);
}

using namespace libyuv;//加载动态库的时候自动执行

#endif //FFMPEG_MASTER_VIDEO_PLAY_H
