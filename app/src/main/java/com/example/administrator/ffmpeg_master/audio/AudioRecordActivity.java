package com.example.administrator.ffmpeg_master.audio;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaRecorder;
import android.net.rtp.AudioCodec;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;

import com.example.administrator.ffmpeg_master.R;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;

import static android.media.AudioFormat.CHANNEL_OUT_STEREO;

@TargetApi(Build.VERSION_CODES.LOLLIPOP)
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

    private String audioPath = Environment.getExternalStorageDirectory() + "/raw/test.mp3";
    int audioIndex = -1;
    MediaExtractor mediaExtractor;
    String audioSavePath = Environment.getExternalStorageDirectory() + "/raw/test.pcm";

    private void getPCMFormAudio() {
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

    private static Handler handler = new Handler(Looper.getMainLooper());

    private void decodeAudio() {

        MediaFormat mediaFormat = mediaExtractor.getTrackFormat(audioIndex);

        try {
            MediaCodec mediaCodec = MediaCodec.createDecoderByType(mediaFormat.getString(MediaFormat.KEY_MIME));
            //用configure(…)方法让它处于Configured状态
            mediaCodec.configure(mediaFormat, null, null, 0);
            //调用start()方法让其处于Executing状态
            mediaCodec.start();
            //开始使用缓冲区处理数据，此时编解码器处于flushed状态
            ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
            ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();
            boolean codeOver = false;
            boolean inputDone = false;
            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();

            FileOutputStream fos = new FileOutputStream(audioSavePath);

            while (!codeOver) {
                if (!inputDone) {
                    for (int i = 0; i < inputBuffers.length; i++) {
                        int inputIndex = mediaCodec.dequeueInputBuffer(0);
                        if (inputIndex >= 0) {
                            //出现输入buff，编解码器就处于Running 状态
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
                                mediaExtractor.advance();
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
                    } else if (outputIndex < 0) {

                    } else {
                        ByteBuffer outputBuffer;
                        if (Build.VERSION.SDK_INT >= 21) {
                            outputBuffer = mediaCodec.getOutputBuffer(outputIndex);
                        } else {
                            outputBuffer = outputBuffers[outputIndex];
                        }
                        chunkPCM = new byte[bufferInfo.size];
                        outputBuffer.get(chunkPCM);
                        outputBuffer.clear();
                        //数据写入文件中
                        fos.write(chunkPCM);
                        fos.flush();
                        mediaCodec.releaseOutputBuffer(outputIndex, false);
                        //编解码结束
                        if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                            mediaExtractor.release();
                            mediaCodec.stop();
                            mediaCodec.release();
                            codeOver = true;
                            decodeDone = true;
                        }
                    }

                }
            }
            fos.close();
            Log.e("mediaCodec", "音频解码完成");
            //pcm文件转音频文件
            handler.post(new Runnable() {
                @Override
                public void run() {
                    pcmToAudioFile();
                }
            });
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    public static final String PCM_PATH = Environment.getExternalStorageDirectory() + "/raw/test.pcm";
    public static final String AAC_PATH = Environment.getExternalStorageDirectory() + "/raw/test.m4a";

    @SuppressLint("WrongConstant")
    private void pcmToAudioFile() {
        try {
            FileInputStream fis = new FileInputStream(PCM_PATH);
            byte[] buffer = new byte[8 * 1024];
            byte[] allAudioBytes;

            int inputIndex = -1;
            ByteBuffer inputBuffer = null;
            int outputIndex = -1;
            ByteBuffer outputBuffer = null;

            byte[] chunkAudio;
            int outBitSize;
            int outPacketSize;

            //配置时指定目标码率和码率控制模式：
            MediaFormat mediaFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, 44100, 2);
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
            mediaFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 500 * 1024);

            MediaCodec mediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            mediaCodec.start();


            ByteBuffer[] encodeInputBuffers = mediaCodec.getInputBuffers();
            ByteBuffer[] encodeOutputBuffers = mediaCodec.getOutputBuffers();
            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();


            FileOutputStream fos = new FileOutputStream(new File(AAC_PATH));
            BufferedOutputStream bos = new BufferedOutputStream(fos, 500 * 1024);
            boolean isReadEnd = false;
            while (!isReadEnd) {
                for (int i = 0; i < encodeInputBuffers.length - 1; i++) {


                    if (fis.read(buffer) != -1) {
                        allAudioBytes = Arrays.copyOf(buffer, buffer.length);
                    } else {
                        isReadEnd = true;
                        break;
                    }
                    inputIndex = mediaCodec.dequeueInputBuffer(-1);
                    inputBuffer = encodeInputBuffers[inputIndex];
                    if (null != inputBuffer) {
                        inputBuffer.clear();
                    }

                    inputBuffer.limit(allAudioBytes.length);
                    inputBuffer.put(allAudioBytes);
                    mediaCodec.queueInputBuffer(inputIndex, 0, allAudioBytes.length, 0, 0);
                }

                outputIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 10000);
                while (outputIndex >= 0) {
                    outBitSize = bufferInfo.size;
                    outPacketSize = outBitSize + 7;
                    outputBuffer = encodeOutputBuffers[outputIndex];
                    outputBuffer.position(bufferInfo.offset);
                    outputBuffer.limit(bufferInfo.offset + outBitSize);

                    chunkAudio = new byte[outPacketSize];

                    addADTSToPacket(chunkAudio, outPacketSize);

                    outputBuffer.get(chunkAudio, 7, outBitSize);
                    outputBuffer.position(bufferInfo.offset);

                    bos.write(chunkAudio, 0, chunkAudio.length);
                    bos.flush();

                    mediaCodec.releaseOutputBuffer(outputIndex, false);

                    outputIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 10000);
                }
            }
            mediaCodec.stop();
            mediaCodec.release();
            fos.close();
            Log.e("mediaCodec", "音频编码完成");
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private void addADTSToPacket(byte[] packet, int packetLen) {
        int profile = 2; // AAC LC
        int freqIdx = 4; // 44.1KHz
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF;
        packet[1] = (byte) 0xF9;
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    public void audioConvert(View view) {
        getPCMFormAudio();
    }

    public void pcmConvertToWav(View view) {
        PcmToWavUtil pcmToWavUtil = new PcmToWavUtil(SAMPLE_RATE_INHZ, CHANNEL_CONFIG, AudioFormat.ENCODING_PCM_16BIT);
        String saveWavFilePath = Environment.getExternalStorageDirectory() + "/pcm_to_wav.wav";
        pcmToWavUtil.pcmToWav(file.getAbsolutePath(), saveWavFilePath);
    }
}
