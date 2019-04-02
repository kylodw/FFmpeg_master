package com.example.administrator.ffmpeg_master.live;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.util.Log;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/02
 * 音频需要参数
 */
public class AudioPusher extends Pusher {
    private final AudioParams audioParams;
    AudioRecord audioRecord;
    private boolean isPushing = false;
    int bufferSizeInBytes;
    private LiveUtil liveUtil;

    public AudioPusher(AudioParams audioParams, LiveUtil liveUtil) {
        this.audioParams = audioParams;
        this.liveUtil = liveUtil;
        //声道布局
        int channelConfig;
        //声道的个数
        if (audioParams.getChannel() == 1) {
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else if (audioParams.getChannel() == 2) {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        } else {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        }
        bufferSizeInBytes = AudioTrack.getMinBufferSize(audioParams.getSampleRateInHz(), channelConfig, AudioFormat.ENCODING_PCM_16BIT);
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC
                , audioParams.getSampleRateInHz(), AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT
                , bufferSizeInBytes);
    }

    @Override
    void startPush() {

        isPushing = true;
        new Thread(new AudioRecodeTask()).start();
    }

    @Override
    void stopPush() {
        isPushing = false;
        audioRecord.stop();
    }

    @Override
    void destroy() {
        audioRecord.release();
    }

    class AudioRecodeTask implements Runnable {

        @Override
        public void run() {
            audioRecord.startRecording();
            while (isPushing) {
                byte[] buffer = new byte[bufferSizeInBytes];
                int len = audioRecord.read(buffer, 0, buffer.length);
                if (len > 0) {
                    liveUtil.sendAudio(buffer,len);
                    //jni层编码
                    Log.d("onPreviewFrame", "音频解码");
                }
            }
        }
    }
}
