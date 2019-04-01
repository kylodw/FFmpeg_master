package com.example.administrator.ffmpeg_master;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Build;
import android.support.annotation.RequiresApi;
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
     *
     * @param input
     * @param output
     */
    public static native void newdecode(String input, String output);

    //视频控制器
    public native void render(String input, Surface surface);

    //音频
    public native void sound(String input, String output);

    public native void play(String input, Surface surface);


    public AudioTrack createAudioTrack(int sampleRateInHz, int nb_channels) {
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        //声道布局
        int channelConfig;
        //声道的个数
        if (nb_channels == 1) {
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else if (nb_channels == 2) {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        } else {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        }
        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);

        AudioTrack audioTrack;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            audioTrack = new AudioTrack(
                    new AudioAttributes.Builder()
                            .setUsage(AudioAttributes.USAGE_MEDIA)
                            .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                            .build(),
                    new AudioFormat.Builder()
                            .setSampleRate(sampleRateInHz)
                            .setEncoding(audioFormat)
                            .setChannelMask(channelConfig)
                            .build(), bufferSizeInBytes, AudioTrack.MODE_STREAM, AudioManager.AUDIO_SESSION_ID_GENERATE

            );
        } else {
            audioTrack = new AudioTrack(
                    AudioManager.STREAM_MUSIC,
                    sampleRateInHz, channelConfig,
                    audioFormat,
                    bufferSizeInBytes, AudioTrack.MODE_STREAM
            );
        }
        return audioTrack;
    }


}
