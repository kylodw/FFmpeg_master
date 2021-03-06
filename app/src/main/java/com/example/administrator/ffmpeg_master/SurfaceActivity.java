package com.example.administrator.ffmpeg_master;

import android.os.Environment;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import java.io.File;

public class SurfaceActivity extends AppCompatActivity {
    private VideoUtil videoUtil;
    private CustomSurfaceView surfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_surface);
        surfaceView = findViewById(R.id.video_view);
        videoUtil = new VideoUtil();
    }

    public void begin(View view) {
        //可以解MP4  mkv，avi，flv的
        new Thread(new Runnable() {
            @Override
            public void run() {
                String input = new File(Environment.getExternalStorageDirectory(), "huge.mp4").getAbsolutePath();
                videoUtil.render(input, surfaceView.getHolder().getSurface());
            }
        }).start();

    }
}
