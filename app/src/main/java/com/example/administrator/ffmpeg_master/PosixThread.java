package com.example.administrator.ffmpeg_master;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/01
 * JNI多线程
 */
public class PosixThread {
    static {
        System.loadLibrary("native-lib");
    }
    public native void pthread();

    public native void init();

    public native void destroy();
}
