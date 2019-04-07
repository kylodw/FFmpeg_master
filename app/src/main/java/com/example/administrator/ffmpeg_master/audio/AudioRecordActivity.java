package com.example.administrator.ffmpeg_master.audio;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.TargetApi;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaRecorder;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;

import com.example.administrator.ffmpeg_master.R;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import static android.media.AudioFormat.CHANNEL_OUT_STEREO;

@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class AudioRecordActivity extends AppCompatActivity {
    private static final int SAMPLE_RATE_INHZ = 44100;
    private static final int CHANNEL_CONFIG = AudioFormat.CHANNEL_IN_STEREO;
    private AudioRecord audioRecord;
    private byte[] bytes;
    private File file;
    private boolean isRecording = false;
    private AudioTrack audioTrack;
    private int bufferSizeInBytes;
    private int output_bufferSizeInBytes;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio2);
        bufferSizeInBytes = AudioTrack.getMinBufferSize(SAMPLE_RATE_INHZ, CHANNEL_CONFIG, AudioFormat.ENCODING_PCM_16BIT);
        /**
         * MediaRecorder.AudioSource.MIC  手机麦克风输入
         * SAMPLE_RATE_INHZ 采样率  目前44100Hz是唯一可以保证兼容所有Android手机的采样率。
         * CHANNEL_CONFIG   ：CHANNEL_IN_MONO（单通道），CHANNEL_IN_STEREO（双通道）
         * 数据位宽  兼容使用 ENCODING_PCM_16BIT
         * bufferSizeInBytes   AudioRecord 内部的音频缓冲区的大小，该缓冲区的值不能低于一帧“音频帧”（Frame）的大小
         *
         */
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC
                , SAMPLE_RATE_INHZ, CHANNEL_CONFIG, AudioFormat.ENCODING_PCM_16BIT
                , bufferSizeInBytes);
        bytes = new byte[bufferSizeInBytes];
        file = new File(Environment.getExternalStorageDirectory(), "audio.pcm");
    }

    public void startRecording(View view) {
        isRecording = true;
        audioRecord.startRecording();
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    FileOutputStream fileOutputStream = new FileOutputStream(file);
                    if (null != fileOutputStream) {
                        while (isRecording) {
                            //尽快取走音频数据
                            int read = audioRecord.read(bytes, 0, bytes.length);
                            if (AudioRecord.ERROR_INVALID_OPERATION != read) {
                                fileOutputStream.write(bytes);
                            }
                        }
                    }
                    fileOutputStream.close();
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).start();

    }

    public void stopRecording(View view) {
        isRecording = false;
        audioRecord.stop();

    }

    public void playAudoTrack(View view) {
        initAudioTtack();
        file = new File(Environment.getExternalStorageDirectory(), "audio.pcm");
        audioTrack.play();
        try {
            final FileInputStream fileInputStream = new FileInputStream(file);
            new Thread(new Runnable() {
                @Override
                public void run() {
                    byte[] tempBuffer = new byte[output_bufferSizeInBytes];

                    try {
                        while (fileInputStream.available() > 0) {
                            int readCount = fileInputStream.read(tempBuffer);
                            if (readCount == AudioTrack.ERROR_INVALID_OPERATION ||
                                    readCount == AudioTrack.ERROR_BAD_VALUE) {
                                continue;
                            }
                            if (readCount != 0 && readCount != -1) {
                                audioTrack.write(tempBuffer, 0, readCount);
                            }
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }).start();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

    public void initAudioTtack() {
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        //声道布局
        int channelConfig = AudioFormat.CHANNEL_IN_STEREO;
        //声道的个数
//        if (nb_channels == 1) {
//            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
//        } else if (nb_channels == 2) {
//            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
//        } else {
//            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
//        }
        output_bufferSizeInBytes = AudioTrack.getMinBufferSize(SAMPLE_RATE_INHZ, channelConfig, audioFormat);


        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            audioTrack = new AudioTrack(
                    new AudioAttributes.Builder()
                            .setUsage(AudioAttributes.USAGE_MEDIA)
                            .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                            .build(),
                    new AudioFormat.Builder()
                            .setSampleRate(SAMPLE_RATE_INHZ)
                            .setEncoding(audioFormat)
                            .setChannelMask(channelConfig)
                            .build(), bufferSizeInBytes, AudioTrack.MODE_STREAM, AudioManager.AUDIO_SESSION_ID_GENERATE

            );
        } else {
            audioTrack = new AudioTrack(
                    AudioManager.STREAM_MUSIC,
                    SAMPLE_RATE_INHZ, channelConfig,
                    audioFormat,
                    bufferSizeInBytes, AudioTrack.MODE_STREAM
            );
        }
    }

    private String audioPath;
    int audioIndex = -1;
    MediaExtractor mediaExtractor;
    String audioSavePath;

    private void getPCMFormAudio(final String audioPath, String auudioSavePath) {
        mediaExtractor = new MediaExtractor();
        boolean hasAudio = false;
        try {
            mediaExtractor.setDataSource(audioPath);
            for (int i = 0; i < mediaExtractor.getTrackCount(); i++) {
                MediaFormat mediaFormat = mediaExtractor.getTrackFormat(i);
                String mimeType = mediaFormat.getString(MediaFormat.KEY_MIME);
                if (mimeType.startsWith("audio")) {
                    audioIndex = i;
                    hasAudio = true;
                    break;
                }
            }
            if (hasAudio) {
                mediaExtractor.selectTrack(audioIndex);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        decodeAudio();
                    }
                }).start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void decodeAudio() {

        MediaFormat mediaFormat = mediaExtractor.getTrackFormat(audioIndex);
        boolean codeOver = false;
        boolean inputDone = false;
        try {
            MediaCodec mediaCodec = MediaCodec.createDecoderByType(mediaFormat.getString(MediaFormat.KEY_MIME));
            mediaCodec.configure(mediaFormat, null, null, 0);
            mediaCodec.start();

            ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
            ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();

            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();

            FileOutputStream fos = new FileOutputStream(audioSavePath);

            while (!codeOver) {
                if (!inputDone) {
                    for (int i = 0; i < inputBuffers.length; i++) {
                        int inputIndex = mediaCodec.dequeueInputBuffer(0);
                        if (inputIndex >= 0) {
                            ByteBuffer byteBuffer = inputBuffers[inputIndex];
                            byteBuffer.clear();
                            int smapleSize = mediaExtractor.readSampleData(byteBuffer, 0);
                            if (smapleSize < 0) {
                                mediaCodec.queueInputBuffer(inputIndex, 0, 0, 0L, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                            } else {
                                bufferInfo.offset = 0;
                                bufferInfo.size = smapleSize;
                                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                                    bufferInfo.flags = MediaCodec.BUFFER_FLAG_KEY_FRAME;
                                } else {
                                    bufferInfo.flags = MediaCodec.BUFFER_FLAG_SYNC_FRAME;
                                }
                                bufferInfo.presentationTimeUs = mediaExtractor.getSampleTime();
                                mediaCodec.queueInputBuffer(inputIndex, bufferInfo.offset, smapleSize, bufferInfo.presentationTimeUs, 0);
                            }
                        }
                    }
                }
            }
            boolean decodeDone = false;
            byte[] chunkPCM;

            while (!decodeDone) {
                int outputIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);
                if (outputIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    decodeDone = true;
                } else if (outputIndex == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                    outputBuffers = mediaCodec.getOutputBuffers();
                } else if (outputIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                    MediaFormat newFormat = mediaCodec.getOutputFormat();
                }else if(outputIndex<0){

                }else{
                    ByteBuffer outputBuffer;
                    if (Build.VERSION.SDK_INT >= 21){
                        outputBuffer = mediaCodec.getOutputBuffer(outputIndex);
                    } else {
                        outputBuffer = outputBuffers[outputIndex];
                    }

                    chunkPCM = new byte[bufferInfo.size];
                    outputBuffer.get(chunkPCM);
                    outputBuffer.clear();

                    fos.write(chunkPCM);//数据写入文件中
                    fos.flush();
                    mediaCodec.releaseOutputBuffer(outputIndex,false);

                    if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0){//编解码结束
                        mediaExtractor.release();
                        mediaCodec.stop();
                        mediaCodec.release();
                        codeOver = true;
                        decodeDone = true;
                    }
                }

            }

        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
