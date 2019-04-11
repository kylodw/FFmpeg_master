package com.example.administrator.ffmpeg_master.video;

import android.util.Log;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/10
 */
public class JfPlayer {
    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("yuv");
        System.loadLibrary("native-lib");
    }

    private String source;
    private int flags = -1;
    private JfOnPreparedListener jfOnPreparedListener;

    public void setSource(String source) {
        this.source = source;
    }

    public void setJfOnPreparedListener(JfOnPreparedListener jfOnPreparedListener) {
        this.jfOnPreparedListener = jfOnPreparedListener;
    }

    public void setFlags(int flags) {
        this.flags = flags;
    }

    /**
     * 采样前的准备条件
     */
    public void onCallPreared() {
        if (jfOnPreparedListener != null) {
            jfOnPreparedListener.onPrepared();
        }
    }

    /**
     * 获取到文件的总时长和当前进度
     *
     * @param type      0子线程  1 主线程
     * @param time_base 文件的总时长
     * @param clock     当前进度
     */
    public void getAudioTime(int type, long time_base, double clock) {
        Log.e("android", "time_base:" + time_base);
        Log.e("android", "clock:" + clock);
    }

    public void start() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                if (flags == 1) {
                    n_start();
                } else if (flags == 2) {
                    pcmStream();
                }

            }
        }).start();
    }

    public void prepared() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepared(source);
            }
        }).start();
    }

    private native void n_prepared(String source);

    private native void n_start();

    public native void n_stop();

    public native void n_pause();

    public native void n_resume();

    public native void pcmLocal();

    public native void pcmStream();

    public native void seek(int current);
}
