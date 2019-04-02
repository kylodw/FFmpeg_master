package com.example.administrator.ffmpeg_master.live;

import android.media.AudioRecord;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/02
 */
public class AudioParams {
    private int sampleRateInHz = 44100;
    private int channel = 1;
//    private int audioSource;
//    int channelConfig, int audioFormat,
//    int bufferSizeInBytes


    public AudioParams() {
    }

    public AudioParams(int sampleRateInHz, int channel) {
        this.sampleRateInHz = sampleRateInHz;
        this.channel = channel;
    }

    public int getSampleRateInHz() {
        return sampleRateInHz;
    }

    public void setSampleRateInHz(int sampleRateInHz) {
        this.sampleRateInHz = sampleRateInHz;
    }

    public int getChannel() {
        return channel;
    }

    public void setChannel(int channel) {
        this.channel = channel;
    }
}
