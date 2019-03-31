package com.example.administrator.ffmpeg_master;

public class VideoUtil {
    static {
        System.loadLibrary("native-lib");
    }
   public static native void newdecode(String input,String output);
}
