package com.example.administrator.ffmpeg_master;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;

import androidx.appcompat.app.AppCompatActivity;

import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.example.administrator.ffmpeg_master.audio.AudioRecordActivity;
import com.example.administrator.ffmpeg_master.camera.Camera2Activity;
import com.example.administrator.ffmpeg_master.live.LiveActivity;
import com.example.administrator.ffmpeg_master.mediaextractor.MediaExtractorActivity;
import com.example.administrator.ffmpeg_master.video.VideoActivity;
import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private RxPermissions rxPermissions = new RxPermissions(this);
    public static final String TAG = "MainActivity";
    private PosixThread posixThread;


    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initFindView();
        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(impleStringFromJNI());
        UUIDUtils.get();
        applyPermission();
        posixThread = new PosixThread();
    }

    private void initFindView() {
        Button mStartAudio = findViewById(R.id.btn_start_audio);
        mStartAudio.setOnClickListener(this);
        Button mMedia = findViewById(R.id.btn_media);
        mMedia.setOnClickListener(this);
        Button button=findViewById(R.id.btn_goto_audio);
        button.setOnClickListener(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @SuppressLint("CheckResult")
    private void applyPermission() {
        rxPermissions.request(Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO)
                .subscribe(new Consumer<Boolean>() {
                    @Override
                    public void accept(Boolean aBoolean) throws Exception {
                        if (aBoolean) {
                            Log.e(TAG, "权限申请成功");
                        } else {
                            Log.e(TAG, "权限申请失败");
                        }
                    }
                });

    }

    public native String stringFromJNI();

    public native String impleStringFromJNI();

    public native int decode(String inputUrl, String outputUrl);


    public native int stream(String input, String output);   //推流


    public native int ffmpegcore(String[] argv);

    public void decodeClick(View view) {
        String folderurl = Environment.getExternalStorageDirectory().getPath();
        String input = folderurl + "/huge.mp4";
        String output = folderurl + "/output_huge.flv";
        File file = new File(input);
        if (!file.exists()) {
            Toast.makeText(this, "文件不存在" + input, Toast.LENGTH_SHORT).show();
            return;
        }
        decode(input, output);
    }

    public void streamClick(View view) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                String folderurl = Environment.getExternalStorageDirectory().getPath();
                String input = folderurl + "/output_flv.flv";
                File file = new File(input);

                if (!file.exists()) {
                    Toast.makeText(MainActivity.this, "不存在文件", Toast.LENGTH_SHORT).show();
                    return;
                }
                String output = "rtmp://47.103.5.187:1935/live/kylodw";
                stream(input, output);
            }
        }).start();
    }

    //    rtmp://202.69.69.180:443/webcast/bshdlive-pc
    public void commandClick(View view) {
//        File file = new File(Environment.getExternalStorageDirectory(), "huge.mp4");

//        File output = new File(Environment.getExternalStorageDirectory() + "/ts", "output.ts");
//        String output = "rtmp://47.103.5.187:1935/live/kylodw";
        File outputFile = new File(Environment.getExternalStorageDirectory(), "output_flv.flv");
//        ffmpeg -re -i 好汉歌.mp4 -c copy -f hls -bsf:v h264_mp4toannexb output.m3u8
//        String[] cmds = {"ffmpeg", "-i", file.getAbsolutePath(), "-vcodec", "copy", "-acodec", "copy", "-f", "mpegts", output.getAbsolutePath()};//输出为ts文件
//        String[] cmds = {"ffmpeg", "-i", file.getAbsolutePath(), "-vcodec", "copy", "-acodec", "copy", "-f", "flv", output};  //推流
        //可以通过这个down flv格式的视频
        String[] cmds = {"ffmpeg", "-i", "rtmp://202.69.69.180:443/webcast/bshdlive-pc", "-acodec", "copy", "-vcodec", "copy", "-f", "flv", outputFile.getAbsolutePath()};
        ffmpegcore(cmds);
        Log.e(TAG, "输出完成");
    }

    public void video_util_click(View view) {

    }

    public void gotoSurface(View view) {
        Intent it = new Intent(this, SurfaceActivity.class);
        startActivity(it);
    }

    public void gotoAudio(View view) {
        Intent it = new Intent(this, AudioActivity.class);
        startActivity(it);
    }

    /**
     * 下载文件
     *
     * @param view
     */
    public void downloadFile(View view) {
        InputStream is = getResources().openRawResource(R.raw.huge);
        File file = new File(Environment.getExternalStorageDirectory(), "huge.mp4");
        FileOutputStream fos = null;
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        if (!file.exists()) {
        }
        try {
            fos = new FileOutputStream(file);
            byte[] buff = new byte[1024];
            int len = 0;
            while ((len = is.read(buff)) != -1) {
                byteArrayOutputStream.write(buff, 0, len);
            }
            byte[] bs = byteArrayOutputStream.toByteArray();
            fos.write(bs);
            byteArrayOutputStream.close();
            is.close();
            fos.flush();
            fos.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                byteArrayOutputStream.close();
                is.close();
                fos.flush();
                fos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
    }

    public void pthreadExample(View view) {
        posixThread.pthread();
    }

    public void pthreadExample2(View view) {
    }

    public void gotoMultThread(View view) {
        Intent it = new Intent(this, MultThreadActivity.class);
        startActivity(it);
    }

    public void gotoLive(View view) {
        Intent it = new Intent(this, LiveActivity.class);
        startActivity(it);
    }

    public void newCamera2(View view) {
        Intent it = new Intent(this, Camera2Activity.class);
        startActivity(it);
    }

    @Override
    public void onClick(View v) {
        Intent it = null;
        switch (v.getId()) {
            case R.id.btn_start_audio:
                it = new Intent(MainActivity.this, AudioRecordActivity.class);
                startActivity(it);
                break;
            case R.id.btn_media:
                it = new Intent(MainActivity.this, MediaExtractorActivity.class);
                startActivity(it);
                break;
            case R.id.btn_goto_audio:
                it = new Intent(MainActivity.this, VideoActivity.class);
                startActivity(it);
                break;
            default:
                break;
        }
    }
}
