//
// Created by Administrator on 2019/4/10.
//
#include "play_call_java.h"
#include "play_ffmpeg.h"
#include "../java_vm.h"


play_call_java *cj = NULL;
play_ffmpeg *pf = NULL;
play_status *status = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_video_JfPlayer_n_1prepared(JNIEnv *env,
                                                                         jobject instance,
                                                                         jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);
// TODO
    if (pf == NULL) {
        if (cj == NULL) {
            cj = new play_call_java(javaVM, env, &instance);
        }
        status = new play_status();
        pf = new play_ffmpeg(status, cj, source);
        pf->prepare();
    }

    env->ReleaseStringUTFChars(source_, source);
}

pthread_t thread_start;

void *startCallback(void *data) {
    play_ffmpeg *ffmpeg = static_cast<play_ffmpeg *>(data);
    ffmpeg->start();
    pthread_exit(&thread_start);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_video_JfPlayer_n_1start(JNIEnv *env,
                                                                      jobject instance) {

    if (pf == NULL) {
        LOGE("play_ffmpeg为空");
        return;
    }
    pthread_create(&thread_start, NULL, startCallback, pf);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_video_JfPlayer_n_1stop(JNIEnv *env,
                                                                     jobject instance) {
    pf->stop();
    if (pf != NULL) {
        //释放资源
        pf->release();
        delete (pf);
        pf = NULL;
        if (cj != NULL) {
            delete (cj);
            cj = NULL;
        }
        if (status != NULL) {
            delete (status);
            status = NULL;
        }
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_video_JfPlayer_n_1pause(JNIEnv *env,
                                                                      jobject instance) {
    pf->play_pause();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_video_JfPlayer_n_1resume(JNIEnv *env,
                                                                       jobject instance) {
    pf->play_resume();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_video_JfPlayer_pcmLocal(JNIEnv *env,
                                                                      jobject instance) {
    if (pf == NULL) {
        if (cj == NULL) {
            cj = new play_call_java(javaVM, env, &instance);
        }
        status = new play_status();
        pf = new play_ffmpeg(status, cj, NULL);
        pf->play_audio_pcm_local();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_video_JfPlayer_pcmStream(JNIEnv *env,
                                                                       jobject instance) {
    if (pf == NULL) {
        LOGE("play_ffmpeg为空");
        return;
    }
    pf->play_audio_pcm_stream();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_video_JfPlayer_seek(JNIEnv *env, jobject instance,
                                                                  jint current) {
    if (pf == NULL) {
        LOGE("play_ffmpeg为空");
        return;
    }
    pf->seek(current);
}
