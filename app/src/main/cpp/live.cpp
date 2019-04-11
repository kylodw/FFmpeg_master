//
// Created by Administrator on 2019/4/2.
//
#include <x264/x264.h>
#include "live.h"
#include "common.h"
#include <malloc.h>
#include <unistd.h>
#include "faac/faac.h"
#include "java_vm.h"


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

x264_picture_t picture_in;

int y_len, u_len, v_len;
x264_t *video_encode_264_handle;
pthread_mutex_t mutex;
pthread_cond_t cond;
unsigned int start_time;
char *rtmp_pth;

int is_pushing = FALSE;
faacEncHandle audio_encode_handle;
unsigned long nInputSamples;
unsigned long nMaxOutputBytes;
jobject jobj_push_native;


/**
 * 相当于java中的回调
 */
void *start_push(void *arg) {
    JNIEnv *push_env;
    javaVM->AttachCurrentThread(&push_env, NULL);
    jclass j_clazz = push_env->GetObjectClass(jobj_push_native);
    jmethodID j_method_id = push_env->GetMethodID(j_clazz, "throwNativeError", "(I)V");
    //1,建立连接
    RTMP *rtmp = RTMP_Alloc();
    if (!rtmp) {
        push_env->CallVoidMethod(jobj_push_native, j_method_id, 101);
        LOGE("%s", "初始化rtmp失败");
        goto end;
    } else{
        push_env->CallVoidMethod(jobj_push_native, j_method_id, 102);
    }
    RTMP_Init(rtmp);
    rtmp->Link.timeout = 5;//可以设置参数

    RTMP_SetupURL(rtmp, rtmp_pth);
    if (!RTMP_SetupURL(rtmp, rtmp_pth)) {
        push_env->CallVoidMethod(jobj_push_native, j_method_id, 103);
        LOGE("%s", "RTMP_SetupURL连接失败");
        goto end;
    } else{
        push_env->CallVoidMethod(jobj_push_native, j_method_id, 104);
    }
    RTMP_EnableWrite(rtmp);
    push_env->CallVoidMethod(jobj_push_native, j_method_id, 109);
    if (!RTMP_Connect(rtmp, NULL)) {
        LOGE("%s", "RTMP_Connect连接失败");
        push_env->CallVoidMethod(jobj_push_native, j_method_id, 105);
        goto end;
    } else {
        push_env->CallVoidMethod(jobj_push_native, j_method_id, 106);
    }
    //拿到时间计时
    start_time = RTMP_GetTime();
    RTMP_ConnectStream(rtmp, 0);
    if (!RTMP_ConnectStream(rtmp, 0)) {
        push_env->CallVoidMethod(jobj_push_native, j_method_id, 107);
        goto end;
    } else{
        push_env->CallVoidMethod(jobj_push_native, j_method_id, 108);
    }

    is_pushing = TRUE;
    add_aac_sequence_header();
    while (is_pushing) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        //取出数据发送
        RTMPPacket *packet = static_cast<RTMPPacket *>(queue_get_first());
        if (packet) {
            queue_delete_first();
            packet->m_nInfoField2 = rtmp->m_stream_id;  //给StreamId设置数据
            int send_result = RTMP_SendPacket(rtmp, packet, TRUE); //放入到rtmp的队列中
            LOGE("send_result %d", send_result);
            if (!send_result) {
//                LOGE("%s", "发送失败");
                RTMPPacket_Free(packet);
                pthread_mutex_unlock(&mutex);
                goto end;
            } else {
//                LOGE("%s", "发送成功!");
            }
            RTMPPacket_Free(packet);
        }
        pthread_mutex_unlock(&mutex);

    }
    end:
    LOGE("%s", "释放资源");
    RTMP_Close(rtmp);
    free(rtmp_pth);
    RTMP_Free(rtmp);
    javaVM->DetachCurrentThread();
    return NULL;
}

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_startPush(JNIEnv *env, jobject instance,
                                                                      jstring url_) {


    jobj_push_native = env->NewGlobalRef(instance);


    const char *rmtp_url = env->GetStringUTFChars(url_, NULL);
    //复制数据到rtmp_path;
    rtmp_pth = static_cast<char *>(malloc(strlen(rmtp_url) + 1));
    memset(rtmp_pth, 0, strlen(rmtp_url) + 1);
    memcpy(rtmp_pth, rmtp_url, strlen(rmtp_url));

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    create_queue();

    //1,建立连接
    //sendPacket
    //2, 启动消费者线程，（从队里中不断拉取packet发送给服务器）
    pthread_t push_customer_t;
    pthread_create(&push_customer_t, NULL, start_push, NULL);


    //释放
    env->ReleaseStringUTFChars(url_, rmtp_url);
}

void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_sendVideo(JNIEnv *env, jobject instance,
                                                                      jbyteArray buffer) {
    //将视频数据转为YUV420P
    //NV21->YUV420P
    jbyte *nv21_buffer_byte = env->GetByteArrayElements(buffer, NULL);
    jbyte *u = reinterpret_cast<jbyte *>(picture_in.img.plane[1]);
    jbyte *v = reinterpret_cast<jbyte *>(picture_in.img.plane[2]);


    memcpy(picture_in.img.plane[0], nv21_buffer_byte, y_len);
    int i = 0;
    for (; i < u_len; ++i) {
        *(u + i) = *(nv21_buffer_byte + y_len + i * 2 + 1);
        *(v + i) = *(nv21_buffer_byte + y_len + i * 2);
    }
    //5  h264编码得到NALU数组
    x264_nal_t *pp_nal = NULL; //NAL  八位 1个禁止位 2个重要位 5个类型位，下面循环要用到
    int n_nal = -1;
    x264_picture_t picture_out;
    int result_encode = x264_encoder_encode(video_encode_264_handle, &pp_nal, &n_nal, &picture_in,
                                            &picture_out);
    LOGE("%d个NAL", n_nal);
    if (result_encode < 0) {
        LOGE("%s", "编码失败");
        return;
    }
    //6 使用rtmp协议将h264编码的视频数据发送给流媒体服务器

    //帧分为关键帧和普通帧，，关键帧要包含SPS，PPS数据

    //SPS NALU 前四个字节的起始码 一个字节的header  后面的都是payload，要读取从起始码的后面读取

    unsigned char sps[100];
    unsigned char pps[100];
    int sps_len, pps_len;
    //清除数组上的数剧
    memset(sps, 0, 100);
    memset(pps, 0, 100);
    picture_in.i_pts += 1;
    i = 0;
    for (; i < n_nal; i++) {
        if (pp_nal[i].i_type == NAL_SPS) {
            sps_len = pp_nal[i].i_payload - 4;
            memcpy(sps, pp_nal[i].p_payload + 4, sps_len);
        } else if (pp_nal[i].i_type == NAL_PPS) {
            pps_len = pp_nal[i].i_payload - 4;
            memcpy(pps, pp_nal[i].p_payload + 4, pps_len);
            //发送序列信息
            add_264_sequence_header(pps, sps, pps_len, sps_len);
        } else {
            //发送帧信息
            add_264_body(pp_nal[i].p_payload, pp_nal[i].i_payload);
        }
    }
    env->ReleaseByteArrayElements(buffer, nv21_buffer_byte, NULL);
}

void add_264_body(unsigned char *buffer, int len) {

    if (buffer[2] == 0x00) {
        buffer += 4;
        len -= 4;
    } else if (buffer[2] == 0x01) {
        buffer += 3;
        len -= 3;
    }


    RTMPPacket *rtmpPacket = (RTMPPacket *) (malloc(sizeof(RTMPPacket)));
    int bodySize = len + 9;
    RTMPPacket_Alloc(rtmpPacket, static_cast<uint32_t>(bodySize));
    RTMPPacket_Reset(rtmpPacket);

    unsigned char *body = reinterpret_cast<unsigned char *>(rtmpPacket->m_body);

    //头信息与上type  获得type
    int type = buffer[0] & 0x1f;
    LOGE("buffer 0位：%c", buffer[0]);
    int i=0;
    if (type == NAL_SLICE_IDR) {  //NAL头信息中后五位，也就是type的5位等于5 进入方法体
        body[i++] = 0x17;   //1 ,innerframe  关键帧  帧内压缩
    } else{
        body[i++] = 0x27; //frame_type  2      2，interframe  帧间压缩
    }
    //fixed 4byte   0x01表示NALU单元
    body[i++] = 0x01;
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    body[i++] = static_cast<unsigned char>((len >> 24) & 0xff);
    body[i++] = static_cast<unsigned char>((len >> 16) & 0xff);
    body[i++] = static_cast<unsigned char>((len >> 8) & 0xff);
    body[i++] = static_cast<unsigned char>((len) & 0xff);

    memcpy(&body[i], buffer, len);

    rtmpPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    rtmpPacket->m_nBodySize = (uint32_t) (bodySize);
    rtmpPacket->m_nTimeStamp = RTMP_GetTime() - start_time;
    rtmpPacket->m_hasAbsTimestamp = 0;
    rtmpPacket->m_nChannel = 0x04; //音视频通道
    rtmpPacket->m_headerType = RTMP_PACKET_SIZE_LARGE;
    //构建出message ，发送
    add_rtmp_packet(rtmpPacket);
}

/**
 * 264的头信息
 */
void add_264_sequence_header(unsigned char *pps, unsigned char *sps, int pps_len, int sps_len) {
    int body_size = 16 + sps_len + pps_len;  //16 H264的标准配置SPS,PPS ，需要16个字节的配置

    RTMPPacket *rtmpPacket = (RTMPPacket *) (malloc(sizeof(RTMPPacket)));
    LOGE("body_size %d", body_size);
    RTMPPacket_Alloc(rtmpPacket, static_cast<uint32_t>(body_size));
    RTMPPacket_Reset(rtmpPacket);

    unsigned char *body = reinterpret_cast<unsigned char *>(rtmpPacket->m_body);


    int i = 0;
    //H264的配置
    body[i++] = 0x17;//FrameType，4bit，帧类型  AVC都可以兼容 （1-keyframe+codeId-7）IFrame, 7: AVC

    //AVC Sequence Header
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;
    //AVCDecoderConfigurationRecord（AVCPacketType == 0，FrameType==1）
    body[i++] = 0x01;
    body[i++] = sps[1];
    body[i++] = sps[2];
    body[i++] = sps[3];

    body[i++] = 0xFF;
    //sps的配置
    body[i++] = 0xE1;
    body[i++] = static_cast<unsigned char>((sps_len >> 8) & 0xff);
    body[i++] = static_cast<unsigned char>(sps_len & 0xff);
    memcpy(&body[i], sps, static_cast<size_t>(sps_len));
    i += sps_len;


    //pps
    body[i++] = 0x01;
    body[i++] = static_cast<unsigned char>((pps_len >> 8) & 0xff);
    body[i++] = static_cast<unsigned char>((pps_len) & 0xff);
    memcpy(&body[i], pps, static_cast<size_t>(pps_len));

    //rtmp协议数据（Message）协议头信息赋值


    rtmpPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    rtmpPacket->m_nBodySize = static_cast<uint32_t>(body_size);
    rtmpPacket->m_nTimeStamp = 0;
    rtmpPacket->m_hasAbsTimestamp = 0;
    rtmpPacket->m_nChannel = 0x04; //音视频通道
    rtmpPacket->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    //构建出message ，发送
    add_rtmp_packet(rtmpPacket);
    LOGE("m_nBodySize%d", rtmpPacket->m_nBodySize);

}

/**
 * 加入队列
 */
void add_rtmp_packet(RTMPPacket *pPacket) {
    pthread_mutex_lock(&mutex);
    if (is_pushing) {
        queue_append_last(pPacket);
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

/**
 * 添加音频数据包
 */
JNIEXPORT void add_aac_body(unsigned char *buffer, int len) {
    int body_size = 2 + len;
    RTMPPacket *packet = static_cast<RTMPPacket *>(malloc(sizeof(RTMPPacket)));
    //RTMPPacket初始化
    RTMPPacket_Alloc(packet, body_size);
    RTMPPacket_Reset(packet);
    unsigned char *body = reinterpret_cast<unsigned char *>(packet->m_body);
    //头信息配置
    /*AF 00 + AAC RAW data*/
    body[0] = 0xA5;//10 5 SoundFormat(4bits):10=AAC,SoundRate(2bits):3=44kHz,SoundSize(1bit):1=16-bit samples,SoundType(1bit):1=Stereo sound
    body[1] = 0x01;//AACPacketType:1表示AAC raw
    memcpy(&body[2], buffer, len); /*spec_buf是AAC raw数据*/
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nTimeStamp = RTMP_GetTime() - start_time;
    add_rtmp_packet(packet);
}

void add_aac_sequence_header() {
//获取aac头信息的长度
    unsigned char *buf;
    unsigned long len; //长度
    faacEncGetDecoderSpecificInfo(audio_encode_handle, &buf, &len);
    int body_size = 2 + len;
    RTMPPacket *packet = static_cast<RTMPPacket *>(malloc(sizeof(RTMPPacket)));
    //RTMPPacket初始化
    RTMPPacket_Alloc(packet, body_size);
    RTMPPacket_Reset(packet);
    unsigned char *body = reinterpret_cast<unsigned char *>(packet->m_body);
    //头信息配置
    /*AF 00 + AAC RAW data*/
    body[0] = 0xA5;//10 5 SoundFormat(4bits):10=AAC,SoundRate(2bits):3=44kHz,SoundSize(1bit):1=16-bit samples,SoundType(1bit):1=Stereo sound
    body[1] = 0x00;//AACPacketType:0表示AAC sequence header
    memcpy(&body[2], buf, len); /*spec_buf是AAC sequence header数据*/
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 0x04;
    packet->m_hasAbsTimestamp = 0;
    packet->m_nTimeStamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    add_rtmp_packet(packet);
    free(buf);

}

/**
 * 发送音频数据
 */
void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_sendAudio(JNIEnv *env, jobject instance,
                                                                      jbyteArray buffer, jint len) {

    int *pcmbuf;
    unsigned char *bitbuf;
    jbyte *b_buffer = env->GetByteArrayElements(buffer, 0);
    pcmbuf = static_cast<int *>(malloc(nInputSamples * sizeof(int)));
    bitbuf = (unsigned char *) malloc(nMaxOutputBytes * sizeof(unsigned char));
    int nByteCount = 0;
    unsigned int nBufferSize = (unsigned int) len / 2;
    unsigned short *buf = (unsigned short *) b_buffer;
    while (nByteCount < nBufferSize) {
        int audioLength = nInputSamples;
        if ((nByteCount + nInputSamples) >= nBufferSize) {
            audioLength = nBufferSize - nByteCount;
        }
        int i;
        for (i = 0; i < audioLength; i++) {//每次从实时的pcm音频队列中读出量化位数为8的pcm数据。
            int s = ((int16_t *) buf + nByteCount)[i];
            pcmbuf[i] = s << 8;//用8个二进制位来表示一个采样量化点（模数转换）
        }
        nByteCount += nInputSamples;
        //利用FAAC进行编码，pcmbuf为转换后的pcm流数据，audioLength为调用faacEncOpen时得到的输入采样数，bitbuf为编码后的数据buff，nMaxOutputBytes为调用faacEncOpen时得到的最大输出字节数
        int byteslen = faacEncEncode(audio_encode_handle, pcmbuf, audioLength,
                                     bitbuf, nMaxOutputBytes);
        if (byteslen < 1) {
            continue;
        }
        add_aac_body(bitbuf, byteslen);//从bitbuf中得到编码后的aac数据流，放到数据队列
    }
    env->ReleaseByteArrayElements(buffer, b_buffer, NULL);
    if (bitbuf)
        free(bitbuf);
    if (pcmbuf)
        free(pcmbuf);
}


JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_stopPush(JNIEnv *env,
                                                                     jobject instance) {
    is_pushing = FALSE;
}

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_release(JNIEnv *env, jobject instance) {
    env->DeleteGlobalRef(jobj_push_native);
}

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_setVideoOptions(JNIEnv *env,
                                                                            jobject instance,
                                                                            jint width, jint height,
                                                                            jint bitrate,
                                                                            jint fps) {
    x264_param_t param;
    //1，参数设置
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    param.i_csp = X264_CSP_I420;  //编码输入的像素格式  YUV420P
    param.i_width = width;
    param.i_height = height;

    param.rc.i_rc_method = X264_RC_CRF;  //恒定码率 CQP恒定质量 ABR 平均码率
    param.rc.i_bitrate = bitrate / 1000;  //kbps
    param.rc.i_vbv_max_bitrate = static_cast<int>(bitrate / 1000 * 1.2);  //瞬时最大码率

    param.i_fps_num = static_cast<uint32_t>(fps);
    param.i_fps_den = 1;
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;
    param.i_threads = 1;
    param.b_vfr_input = 0; //码率控制，不通过时间基,通过fps来进行码率控制
    param.b_repeat_headers = 1;//是否把SPS和PPS放入每一个keyframe，为了提高图像的纠错能力
    param.i_level_idc = 51;  //level和profile一起控制码率


    y_len = width * height;
    u_len = y_len / 4;
    v_len = u_len;


    //2 设置profile档次
    x264_param_apply_profile(&param, "baseline"); //没有b帧
    //3 输入图像初始化

    x264_picture_alloc(&picture_in, param.i_csp, param.i_width, param.i_height);
    picture_in.i_pts = 0;
    //4 打开编码器
    video_encode_264_handle = x264_encoder_open(&param);
    if (!video_encode_264_handle) {
        LOGE("x264_t%s", "编码失败");
    } else {
        LOGE("x264_t%s", "编码器成功");
    }


}

JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_live_LiveUtil_setAudioOptions(JNIEnv *env,
                                                                            jobject instance,
                                                                            jint sampleRateInHz,
                                                                            jint channel) {

    audio_encode_handle = faacEncOpen((unsigned long) sampleRateInHz, (unsigned long) channel,
                                      &nInputSamples, &nMaxOutputBytes);
    if (!audio_encode_handle) {
        LOGE("%s", "音频编码器打开失败");
        return;
    }
    faacEncConfigurationPtr faac_ptr_config = faacEncGetCurrentConfiguration(audio_encode_handle);
    faac_ptr_config->mpegVersion = MPEG4;
    faac_ptr_config->aacObjectType = LOW;
    faac_ptr_config->outputFormat = 0;
    faac_ptr_config->useTns = 1;
    faac_ptr_config->useLfe = 0;
//    faac_ptr_config->inputFormat = FAAC_INPUT_16BIT;
    faac_ptr_config->quantqual = 100;
    faac_ptr_config->bandWidth = 0;
    faac_ptr_config->shortctl = SHORTCTL_NORMAL;
    if (!faacEncSetConfiguration(audio_encode_handle, faac_ptr_config)) {
        LOGE("%s", "音频编码器配置失败");
    } else {
        LOGE("%s", "音频编码器配置成功");
    }
    LOGE("%s", "音频打开完成");


}

