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
        status=new play_status();
        pf = new play_ffmpeg(status,cj, source);
        pf->prepare();
    }

    env->ReleaseStringUTFChars(source_, source);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_video_JfPlayer_n_1start(JNIEnv *env,
                                                                      jobject instance) {

    // TODO
    pf->start();

}