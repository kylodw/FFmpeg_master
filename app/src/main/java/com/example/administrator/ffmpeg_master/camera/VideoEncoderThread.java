package com.example.administrator.ffmpeg_master.camera;

import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Build;
import android.util.Log;

import com.example.administrator.ffmpeg_master.util.CodecUtil;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.util.Vector;

/**
 * 视频编码线程
 */
@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class VideoEncoderThread extends Thread {


    private static final String TAG = "VideoEncoderThread";

    // 编码相关参数
    private static final String MIME_TYPE = "video/avc"; // H.264 Advanced Video
    private static final int FRAME_RATE = 25; // 帧率
    private static final int IFRAME_INTERVAL = 1; // I帧间隔（GOP）
    private static final int TIMEOUT_USEC = 0; // 编码超时时间

    // 视频宽高参数
    private int mWidth;
    private int mHeight;

    // 存储每一帧的数据 Vector 自增数组
    private Vector<byte[]> frameBytes;
    private byte[] mFrameData;

    private static final int COMPRESS_RATIO = 256;
    private int BIT_RATE;

    private final Object lock = new Object();

    private MediaCodecInfo mCodecInfo;
    private MediaCodec mMediaCodec;
    private MediaCodec.BufferInfo mBufferInfo;

    private WeakReference<MediaMuxerThread> mediaMuxer;
    private MediaFormat mediaFormat;

    private volatile boolean isStart = false;
    private volatile boolean isExit = false;
    private volatile boolean isMuxerReady = false;


    public VideoEncoderThread(int mWidth, int mHeight, WeakReference<MediaMuxerThread> mediaMuxer) {
        // 初始化相关对象和参数
        this.mWidth = mWidth;
        this.mHeight = mHeight;
        this.mediaMuxer = mediaMuxer;
        frameBytes = new Vector<byte[]>();
//        BIT_RATE=mWidth*mHeight* 3 * 8 * FRAME_RATE / COMPRESS_RATIO;
        BIT_RATE = mWidth * mHeight * 5;
        Log.e(TAG, "bitl率：" + mWidth * mHeight * 3 * 8 * FRAME_RATE / COMPRESS_RATIO);
        prepare();
    }

    // 执行相关准备工作

    private void prepare() {
        Log.i(TAG, "VideoEncoderThread().prepare");
        mFrameData = new byte[this.mWidth * this.mHeight * 3 / 2];
        mBufferInfo = new MediaCodec.BufferInfo();
        mCodecInfo = selectCodec(MIME_TYPE);
        if (mCodecInfo == null) {
            Log.e(TAG, "Unable to find an appropriate codec for " + MIME_TYPE);
            return;
        }
        mediaFormat = MediaFormat.createVideoFormat(MIME_TYPE, this.mWidth, this.mHeight);
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, BIT_RATE);
        mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, FRAME_RATE);
        //这种颜色格式从来都不是一个确定的格式，只是代表YUV420这一类格式
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, IFRAME_INTERVAL);
    }

    private static MediaCodecInfo selectCodec(String mimeType) {
        int numCodecs = MediaCodecList.getCodecCount();
        for (int i = 0; i < numCodecs; i++) {
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
            if (!codecInfo.isEncoder()) {
                continue;
            }
            String[] types = codecInfo.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase(mimeType)) {
                    return codecInfo;
                }
            }
        }
        return null;
    }


    /**
     * 开始视频编码
     *
     * @throws IOException
     */
    private void startMediaCodec() throws IOException {
        mMediaCodec = MediaCodec.createByCodecName(mCodecInfo.getName());
        Log.e("startMediaCodec", "name:" + mCodecInfo.getName());
        mMediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        mMediaCodec.start();
        isStart = true;
    }

    public void setMuxerReady(boolean muxerReady) {
        synchronized (lock) {
            Log.e(TAG, Thread.currentThread().getId() + " video -- setMuxerReady..." + muxerReady);
            isMuxerReady = muxerReady;
            lock.notifyAll();
        }
    }

    public void add(byte[] data) {
        if (frameBytes != null && isMuxerReady) {
            frameBytes.add(data);
        }
    }

    public synchronized void restart() {
        isStart = false;
        isMuxerReady = false;
        frameBytes.clear();
    }

    @Override
    public void run() {
        super.run();

        while (!isExit) {
            if (!isStart) {
                stopMediaCodec();

                if (!isMuxerReady) {
                    synchronized (lock) {
                        try {
                            Log.e(TAG, "video -- 等待混合器准备...");
                            lock.wait();
                        } catch (InterruptedException e) {
                        }
                    }
                }

                if (isMuxerReady) {
                    try {
                        Log.e(TAG, "video -- startMediaCodec...");
                        startMediaCodec();
                    } catch (IOException e) {
                        isStart = false;
                        try {
                            Thread.sleep(100);
                        } catch (InterruptedException e1) {
                        }
                    }
                }

            } else if (!frameBytes.isEmpty()) {
                byte[] bytes = this.frameBytes.remove(0);
                Log.e("ang-->", "解码视频数据:" + bytes.length);
                try {
                    encodeFrame(bytes);
                } catch (Exception e) {
                    Log.e(TAG, "解码视频(Video)数据 失败");
                    e.printStackTrace();
                }
            }
        }
        Log.e(TAG, "Video 录制线程 退出...");
    }

    public void exit() {
        isExit = true;
    }

    /**
     * 编码每一帧的数据
     *
     * @param input 每一帧的数据
     */
    private void encodeFrame(byte[] input) {
        Log.w(TAG, "VideoEncoderThread.encodeFrame()");
//        yuv420pTo420sp(input, mFrameData, this.mWidth, this.mHeight);

        // 将原始的N21数据转为I420
//        YV12toNV12(input, mFrameData, this.mWidth, this.mHeight);
//        NV21toI420SemiPlanar(input, mFrameData, this.mWidth, this.mHeight);
//        ByteBuffer[] inputBuffers = mMediaCodec.getInputBuffers();
//        ByteBuffer[] outputBuffers = mMediaCodec.getOutputBuffers();
//        input = nv21ToI420(input, this.mWidth, this.mHeight);

        int inputBufferIndex = mMediaCodec.dequeueInputBuffer(TIMEOUT_USEC);
        if (inputBufferIndex >= 0) {
            ByteBuffer inputBuffer = CodecUtil.getInputBuffer(mMediaCodec, inputBufferIndex);
            inputBuffer.clear();
            inputBuffer.put(input);
            mMediaCodec.queueInputBuffer(inputBufferIndex, 0, input.length, System.nanoTime() / 1000, 0);
        } else {
            Log.e(TAG, "input buffer not available");
        }

        int outputBufferIndex = mMediaCodec.dequeueOutputBuffer(mBufferInfo, TIMEOUT_USEC);
        Log.i(TAG, "outputBufferIndex-->" + outputBufferIndex);
        do {
            if (outputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
            } else if (outputBufferIndex == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                Log.e(TAG, "进入INFO_OUTPUT_BUFFERS_CHANGED");
            } else if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                MediaFormat newFormat = mMediaCodec.getOutputFormat();
                MediaMuxerThread mediaMuxerRunnable = this.mediaMuxer.get();
                if (mediaMuxerRunnable != null) {
                    mediaMuxerRunnable.addTrackIndex(MediaMuxerThread.TRACK_VIDEO, newFormat);
                }
            } else if (outputBufferIndex < 0) {
                Log.e(TAG, "outputBufferIndex < 0");
            } else {
                Log.d(TAG, "perform encoding");
                ByteBuffer outputBuffer = CodecUtil.getOutputBuffer(mMediaCodec, outputBufferIndex);
                if (outputBuffer == null) {
                    throw new RuntimeException("encoderOutputBuffer " + outputBufferIndex + " was null");
                }
                if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                    Log.d(TAG, "ignoring BUFFER_FLAG_CODEC_CONFIG");
                    mBufferInfo.size = 0;
                }
                if (mBufferInfo.size != 0) {
                    MediaMuxerThread mediaMuxer = this.mediaMuxer.get();

                    if (mediaMuxer != null && !mediaMuxer.isVideoTrackAdd()) {
                        MediaFormat newFormat = mMediaCodec.getOutputFormat();
                        mediaMuxer.addTrackIndex(MediaMuxerThread.TRACK_VIDEO, newFormat);
                    }
                    // adjust the ByteBuffer values to match BufferInfo (not needed?)
                    outputBuffer.position(mBufferInfo.offset);
                    outputBuffer.limit(mBufferInfo.offset + mBufferInfo.size);

                    if (mediaMuxer != null && mediaMuxer.isMuxerStart()) {
                        mediaMuxer.addMuxerData(new MediaMuxerThread.MuxerData(MediaMuxerThread.TRACK_VIDEO, outputBuffer, mBufferInfo));
                    }

                    Log.d(TAG, "sent " + mBufferInfo.size + " frameBytes to muxer");
                }
                mMediaCodec.releaseOutputBuffer(outputBufferIndex, false);
            }
            outputBufferIndex = mMediaCodec.dequeueOutputBuffer(mBufferInfo, TIMEOUT_USEC);
        } while (outputBufferIndex >= 0);
    }

    /**
     * 停止视频编码
     */
    private void stopMediaCodec() {
        if (mMediaCodec != null) {
            mMediaCodec.stop();
            mMediaCodec.release();
            mMediaCodec = null;
        }
        isStart = false;
        Log.e(TAG, "stop video 录制...");
    }


    private static void NV21toI420SemiPlanar(byte[] nv21bytes, byte[] i420bytes, int width, int height) {
        System.arraycopy(nv21bytes, 0, i420bytes, 0, width * height);
        for (int i = width * height; i < nv21bytes.length; i += 2) {
            i420bytes[i] = nv21bytes[i + 1];
            i420bytes[i + 1] = nv21bytes[i];
        }
    }

    private void yuv420pTo420sp(byte[] yuv420p, byte[] yuv420sp, int width, int height) {
        if (yuv420p == null || yuv420sp == null) return;
        int frameSize = width * height;
        int j;
        System.arraycopy(yuv420p, 0, yuv420sp, 0, frameSize);
        for (j = 0; j < frameSize / 4; j++) {
            // u
            yuv420sp[frameSize + 2 * j] = yuv420p[j + frameSize];
            // v
            yuv420sp[frameSize + 2 * j + 1] = yuv420p[(int) (j + frameSize * 1.25)];
        }
    }

    private void YV12toNV12(byte[] yv12bytes, byte[] nv12bytes, int width, int height) {

        int nLenY = width * height;
        int nLenU = nLenY / 4;


        System.arraycopy(yv12bytes, 0, nv12bytes, 0, width * height);
        for (int i = 0; i < nLenU; i++) {
            nv12bytes[nLenY + 2 * i] = yv12bytes[nLenY + i];
            nv12bytes[nLenY + 2 * i + 1] = yv12bytes[nLenY + nLenU + i];
        }
    }

    public static byte[] nv21ToI420(byte[] data, int width, int height) {
        byte[] ret = new byte[data.length];
        int total = width * height;

        ByteBuffer bufferY = ByteBuffer.wrap(ret, 0, total);
        ByteBuffer bufferU = ByteBuffer.wrap(ret, total, total / 4);
        ByteBuffer bufferV = ByteBuffer.wrap(ret, total + total / 4, total / 4);

        bufferY.put(data, 0, total);
        for (int i = total; i < data.length; i += 2) {
            bufferV.put(data[i]);
            bufferU.put(data[i + 1]);
        }

        return ret;
    }
}
