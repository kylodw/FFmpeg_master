package com.example.administrator.ffmpeg_master.live;

import android.Manifest;
import android.annotation.SuppressLint;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import com.example.administrator.ffmpeg_master.R;
import com.tbruyelle.rxpermissions2.RxPermissions;

import io.reactivex.functions.Consumer;

/**
 * 音视频采集
 */
public class LiveActivity extends AppCompatActivity {
    private SurfaceView surfaceView;
    private RxPermissions rxPermissions;

    LivePusher livePusher;
    static final String URL="rtmp地址";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_live);
        surfaceView = findViewById(R.id.surface_view);
        livePusher = new LivePusher(surfaceView.getHolder());
        applyPermission();
    }

    @SuppressLint("CheckResult")
    private void applyPermission() {
        rxPermissions = new RxPermissions(this);
        rxPermissions.request(Manifest.permission.CAMERA, Manifest.permission.RECORD_AUDIO)
                .subscribe(new Consumer<Boolean>() {
                    @Override
                    public void accept(Boolean aBoolean) throws Exception {
                    }
                });

    }

    public void startLive(View view) {
        Button button = (Button) view;
        if (button.getText().equals("开始直播")) {
            livePusher.startPush(URL);
            button.setText("停止直播");
        } else {
            livePusher.stopPush();
            button.setText("开始直播");
        }

    }

    public void switchCamera(View view) {
        livePusher.switchCamera();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}
