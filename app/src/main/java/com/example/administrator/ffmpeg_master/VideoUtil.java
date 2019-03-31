package com.example.administrator.ffmpeg_master;

import android.view.Surface;
import android.view.SurfaceView;

public class VideoUtil {
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

    /**
     * 解码
     * @param input
     * @param output
     */
   public static native void newdecode(String input,String output);

   //视频控制器
   public native void render(String input,Surface surface);
}
