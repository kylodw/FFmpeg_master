package com.example.administrator.ffmpeg_master.mediaextractor;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.Toast;

import com.example.administrator.ffmpeg_master.R;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR2)
public class MediaExtractorActivity extends AppCompatActivity {
    public static final String URL = Environment.getExternalStorageDirectory() + "/ts";
    private MediaExtractor mediaExtractor;
    private MediaMuxer mediaMuxer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_media_extractor);

    }


    private void exactorMedia() {
        mediaExtractor = new MediaExtractor();

        FileOutputStream video_output_stream = null;
        FileOutputStream audio_output_stream = null;

        File videoFile = new File(URL, "output.mp4");
        File audioFile = new File(URL, "output_audio.pcm");

        try {
            video_output_stream = new FileOutputStream(videoFile);
            audio_output_stream = new FileOutputStream(audioFile);

            File file = new File(Environment.getExternalStorageDirectory(), "huge.mp4");
            if (!file.exists()) {
                Toast.makeText(this, "文件不存在,请先下载文件", Toast.LENGTH_SHORT).show();
                return;
            }
            String input_url = Environment.getExternalStorageDirectory() + "/huge.mp4";
            mediaExtractor.setDataSource(input_url);
            //信道总数
            int trackCount = mediaExtractor.getTrackCount();
            int audioIndex = -1;
            int videoIndex = -1;

            for (int i = 0; i < trackCount; i++) {
                MediaFormat mediaFormat = mediaExtractor.getTrackFormat(i);
                String mineType = mediaFormat.getString(MediaFormat.KEY_MIME);
                if (!mineType.startsWith("video")) {
                    continue;
                }
                mediaMuxer = new MediaMuxer(videoFile.getAbsolutePath(), MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
                mediaExtractor.selectTrack(i);
                videoIndex = mediaMuxer.addTrack(mediaFormat);
                mediaMuxer.start();
            }

            MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
            bufferInfo.presentationTimeUs = 0;
            ByteBuffer byteBuffer = ByteBuffer.allocate(25 * 1024 * 1024);

            while (true) {
                int readSmapleCount = mediaExtractor.readSampleData(byteBuffer, 0);
                if (readSmapleCount < 0) {
                    break;
                }
                bufferInfo.offset = 0;
                bufferInfo.size = readSmapleCount;
                bufferInfo.flags = mediaExtractor.getSampleFlags();
                bufferInfo.presentationTimeUs = mediaExtractor.getSampleTime();
                mediaMuxer.writeSampleData(videoIndex, byteBuffer, bufferInfo);
                mediaExtractor.advance();
            }
//              while (true) {
//                int readSmapleCount = mediaExtractor.readSampleData(byteBuffer, 0);
//                if (readSmapleCount < 0) {
//                    break;
//                }
//                bufferInfo.offset = 0;
//                bufferInfo.size = readSmapleCount;
//                bufferInfo.flags = mediaExtractor.getSampleFlags();
//                bufferInfo.presentationTimeUs = mediaExtractor.getSampleTime();
//
////                byte[] bytes = new byte[readSmapleCount];
////                byteBuffer.get(bytes);
////                video_output_stream.write(bytes);
////                byteBuffer.clear();
//                mediaMuxer.writeSampleData(audioIndex, byteBuffer, bufferInfo);
//                mediaExtractor.advance();
//            }


        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            mediaExtractor.release();
            if (null != mediaMuxer) {
                mediaMuxer.stop();
                mediaMuxer.release();
            }
            try {
                if (video_output_stream != null) {
                    video_output_stream.close();
                }
                if (audio_output_stream != null) {
                    audio_output_stream.close();
                }

            } catch (IOException e) {
                e.printStackTrace();
            }

        }

    }

    public void separationVideo(View view) {
        exactorMedia();
    }
}
