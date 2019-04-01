package com.example.administrator.ffmpeg_master;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.example.administrator.ffmpeg_master.util.CmdUtil;
import com.tbruyelle.rxpermissions2.RxPermissions;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity {
    private RxPermissions rxPermissions;
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

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(impleStringFromJNI());
        UUIDUtils.get();
        applyPermission();
        posixThread = new PosixThread();

//        decode(folderurl + "/" + "test.mp4", folderurl + "/" + "output.yuv");
    }

    @SuppressLint("CheckResult")
    private void applyPermission() {
        rxPermissions = new RxPermissions(this);
        rxPermissions.request(Manifest.permission.WRITE_EXTERNAL_STORAGE)
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

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native String impleStringFromJNI();

    public native int decode(String inputUrl, String outputUrl);


    public native int stream(String input, String output);   //推流


    public native int ffmpegcore(String[] argv);

    public void decodeClick(View view) {
        String folderurl = Environment.getExternalStorageDirectory().getPath();
        String input = folderurl + "/123.mp4";
        String output = folderurl + "/output.yuv";
        File file = new File(input);
        if (!file.exists()) {
            Toast.makeText(this, "文件不存在" + input, Toast.LENGTH_SHORT).show();
            return;
        }
        decode(input, output);
        Log.e("Main", folderurl);
    }

    public void streamClick(View view) {
        Toast.makeText(this, "推流得好好整理", Toast.LENGTH_SHORT).show();

//        String folderurl = Environment.getExternalStorageDirectory().getPath();
//        String input = folderurl + "/test.mp4";
//        String output = "rtmp://192.168.31.126:1935/live/livestream";
//        decode(input, output);
//        Log.e("Main", folderurl);
    }

    public void commandClick(View view) {
        CmdUtil.getCmd();
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

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    public void pthreadExample2(View view) {
    }

    public void gotoMultThread(View view) {
        Intent it = new Intent(this, MultThreadActivity.class);
        startActivity(it);
    }
}
