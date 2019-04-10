package com.example.administrator.ffmpeg_master.video;

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
    private JfOnPreparedListener jfOnPreparedListener;

    public void setSource(String source) {
        this.source = source;
    }

    public void setJfOnPreparedListener(JfOnPreparedListener jfOnPreparedListener) {
        this.jfOnPreparedListener = jfOnPreparedListener;
    }

    public void onCallPreared() {
        if (jfOnPreparedListener != null) {
            jfOnPreparedListener.onPrepared();
        }
    }

    public void start() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
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

}
