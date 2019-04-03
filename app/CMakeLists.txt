# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)


set(CURRENT_DIR ${CMAKE_SOURCE_DIR})
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/native-lib.cpp
        #        #编译额外的c文件
        src/main/jni/cmdutils.c
        src/main/jni/ffmpeg_filter.c
        src/main/jni/ffmpeg_mod.c
        src/main/jni/ffmpeg_opt.c
        src/main/cpp/ffmpeg_decode_k.cpp
        src/main/cpp/video_play.cpp
        src/main/cpp/queue.cpp
        src/main/cpp/live.cpp
        src/main/cpp/queue_live.c
#        src/main/cpp/c_k_pthread.cpp

        )


#FFmpeg配置目录
set(distribution_DIR src/main/jniLibs)
#编解码
add_library(
        avcodec
        SHARED
        IMPORTED)
set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavcodec-57.so)
#设备信息
add_library(
        avdevice
        SHARED
        IMPORTED)
set_target_properties(
        avdevice
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavdevice-57.so)

#滤镜特效
add_library(
        avfilter
        SHARED
        IMPORTED)
set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavfilter-6.so)

#封装格式
add_library(
        avformat
        SHARED
        IMPORTED)
set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavformat-57.so)


#工具库
add_library(
        avutil
        SHARED
        IMPORTED)
set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavutil-55.so)

#后期处理
add_library(
        postproc
        SHARED
        IMPORTED)
set_target_properties(
        postproc
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libpostproc-54.so)


#音频采样数据格式转换
add_library(
        wresample
        SHARED
        IMPORTED)
set_target_properties(
        wresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswresample-2.so)
#视频采样
add_library(
        wscale
        SHARED
        IMPORTED)
set_target_properties(
        wscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswscale-4.so)

#yuv格式转换
add_library(
        yuv
        SHARED
        IMPORTED)
set_target_properties(
        yuv
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libyuv.so)
#x264
add_library(
        x264
        STATIC
        IMPORTED)
set_target_properties(
        x264
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libx264.a)

#faac
add_library(
        faac
        STATIC
        IMPORTED)
set_target_properties(
        faac
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libfaac.a)

#yuv格式转换
add_library(
        rtmp
        SHARED
        IMPORTED)
set_target_properties(
        rtmp
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/librtmp.so)


include_directories(src/main/jniLibs/include)
include_directories(src/main/jniLibs/include/other)
include_directories(src/main/jniLibs/include/yuv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#add_definitions(-Wall)


#set(CMAKE_CXX_COMPILER "clang++")
#other
#if(CMAKE_COMPILER_IS_GUNCXX)
#    set(CMAKE_CXX_FLAGS "-std=c++11 -w ${CMAKE_CXX_FLAGS}")
#    message(STATUS "optional:-std=c++11")
#endif(CMAKE_COMPILER_IS_GUNCXX)

add_compile_options(-w)
# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        log
        )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        #连接额外的ffmpeg

        native-lib avcodec avdevice avfilter avformat avutil postproc wresample wscale yuv x264 faac rtmp
        # Links the target library to the log library
        # included in the NDK.
        -landroid # Add this.
        ${log-lib}
        )