package com.example.administrator.ffmpeg_master.live;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/02
 */
public class LiveUtil {

    LiveListener liveListener;

    public void setLiveListener(LiveListener liveListener) {
        this.liveListener = liveListener;
    }

    public void removeLiveListener() {
        this.liveListener = null;
    }

    static {
        System.loadLibrary("rtmp");
        System.loadLibrary("native-lib");
    }

    public native void startPush(String url);

    public native void stopPush();

    public native void release();

    public native void setVideoOptions(int width, int height, int bitrate, int fps);

    public native void setAudioOptions(int sampleRateInHz, int channel);

    public void throwNativeError(int code) {
        if (liveListener != null) {
            liveListener.onError(code);
        }
    }

    /**
     * 发送视频
     *
     * @param data
     */
    public native void sendVideo(byte[] data);

    /**
     * 发送音频
     *
     * @param data
     * @param len
     */
    public native void sendAudio(byte[] data, int len);
}
