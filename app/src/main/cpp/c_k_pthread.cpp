//
// Created by Administrator on 2019/4/1.
//
#include <jni.h>
#include <pthread.h>
#include <stdio.h>
#include <android/log.h>
#include <unistd.h>

#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "这是loge", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "这是logi", format, ##__VA_ARGS__)

static const char *const clazz_name = "com/example/administrator/ffmpeg_master/UUIDUtils";

void *th_fun(void *arg);



JavaVM *javaVM;
jobject uuidutil_class_global;
jmethodID uuid_get_method_id;

//加载动态库的时候自动执行
extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGE("%s", "JNILOAD");
    javaVM = vm;
    //兼容AndroidSDk2.2之后
    return JNI_VERSION_1_4;
}



void *th_fun(void *arg) {

    char *no = (char *) (arg);
    int i = 0;
    for (; i < 5; ++i) {
        JNIEnv *env;
//    //获取JNIEnv,java_vm关联当前线程
        javaVM->AttachCurrentThread(&env, NULL);
        JavaVMAttachArgs args[] = {JNI_VERSION_1_4, "my_thread", NULL};
        LOGE("thread %s , 当前i:%d", no, i);
        jstring uuid = (jstring) (env->CallStaticObjectMethod((jclass) uuidutil_class_global,
                                                              uuid_get_method_id));
        const char *uuid_string = env->GetStringUTFChars(uuid, NULL);
        LOGE("这是UUID:%s", uuid_string);
        if (i == 4) {
            goto end;
        }
        env->ReleaseStringUTFChars((jstring) uuid, uuid_string);
        sleep(1);
    }
    end:
    javaVM->DetachCurrentThread();
    pthread_exit(0);
}


//JavaVm 代表的java虚拟机，所有访问对象的工作都是这个
//JavaVm可以拿到单独线程关联的JNIEnv
//每个线程都有一个独立的JNIEnv
//获取javaVM
extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_PosixThread_pthread(JNIEnv *env, jobject instance) {
//    jint vm_version = env->GetJavaVM(&javaVM);
//    LOGE("java_vm版本%d", vm_version);
    //创建多线程
    pthread_t tid;
    pthread_create(&tid, NULL, th_fun, (void *) "No1");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_PosixThread_init(JNIEnv *env, jobject instance) {
    //在子线程中findclass
    jclass uuidutil_class_temp = env->FindClass(clazz_name);
    uuidutil_class_global = env->NewGlobalRef(uuidutil_class_temp);
    //有分号代表引用
    uuid_get_method_id = env->GetStaticMethodID((jclass) (uuidutil_class_global), "get",
                                                "()Ljava/lang/String;");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_administrator_ffmpeg_1master_PosixThread_destroy(JNIEnv *env, jobject instance) {

    // TODO
    env->DeleteGlobalRef(uuidutil_class_global);

}