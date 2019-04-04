package com.example.administrator.ffmpeg_master.live;

import android.os.Handler;
import android.os.Message;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.example.administrator.ffmpeg_master.R;
import com.example.administrator.ffmpeg_master.live.pusher.LivePusher;
import com.tbruyelle.rxpermissions2.RxPermissions;

/**
 * 音视频采集
 */
public class LiveActivity extends AppCompatActivity implements LiveListener {
    private SurfaceView surfaceView;
    private RxPermissions rxPermissions;

    LivePusher livePusher;
    //for test
    static final String URL = "rtmp://47.103.5.187:1935/live/kylodw";
    private TextView mTv1;
    private TextView mTv2;
    private TextView mTv3;
    private TextView mTv4;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_live);
        surfaceView = findViewById(R.id.surface_view);
        livePusher = new LivePusher(surfaceView.getHolder(), this);
        mTv1 = findViewById(R.id.tv_1);
        mTv2 = findViewById(R.id.tv_2);
        mTv3 = findViewById(R.id.tv_3);
        mTv4 = findViewById(R.id.tv_4);
    }

    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case 101:
                    mTv1.setText("初始化rtmp失败");
                    break;
                case 102:
                    mTv1.setText("初始化rtmp成功");
                    break;
                case 103:
                    mTv2.setText("RTMP_SetupURL连接失败");
                    break;
                case 104:
                    mTv2.setText("RTMP_SetupURL连接成功");
                    break;
                case 105:
                    mTv3.setText("RTMP_Connect连接失败");
                    break;
                case 106:
                    mTv3.setText("RTMP_Connect连接成功");
                    break;
                case 107:
                    mTv4.setText("RTMP_ConnectStream连接失败");
                    break;
                case 108:
                    mTv4.setText("RTMP_ConnectStream连接成功");
                    break;
                case 109:
//                    进入RTMP_EnableWrite
                    break;
            }
        }
    };

    public void startLive(View view) {
        Button button = (Button) view;
        if (button.getText().equals("开始直播")) {
            livePusher.startPush(URL, this);
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

    @Override
    public void onError(int code) {
        handler.sendEmptyMessage(code);
    }
}
