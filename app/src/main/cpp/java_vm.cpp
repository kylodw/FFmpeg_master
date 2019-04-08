//
// Created by Administrator on 2019/4/8.
//

#include "java_vm.h"

JavaVM * javaVM;
extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
//兼容AndroidSDk2.2之后
    return JNI_VERSION_1_4;
}