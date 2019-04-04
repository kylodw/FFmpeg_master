package com.example.administrator.ffmpeg_master;

import android.os.Environment;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import java.io.File;

public class MultThreadActivity extends AppCompatActivity {
    private CustomSurfaceView surfaceView;
    private VideoUtil videoUtil;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mult_thread);
        surfaceView = findViewById(R.id.video_view);
        videoUtil = new VideoUtil();
    }

    public void beginDecode(View view) {
        //可以解MP4  mkv，avi，flv的
        String input = new File(Environment.getExternalStorageDirectory(), "huge.mp4").getAbsolutePath();
        videoUtil.play(input, surfaceView.getHolder().getSurface());
    }
}
