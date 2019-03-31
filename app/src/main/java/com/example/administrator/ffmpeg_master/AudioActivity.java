package com.example.administrator.ffmpeg_master;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import java.io.File;

public class AudioActivity extends AppCompatActivity {
    private VideoUtil videoUtil;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio);
        videoUtil=new VideoUtil();
    }

    public void audio_stop(View view) {
    }

    public void audio_begin(View view) {
        String input = new File(Environment.getExternalStorageDirectory(),"huge.mp4").getAbsolutePath();
        String output = new File(Environment.getExternalStorageDirectory(),"out_put_bofang.pcm").getAbsolutePath();
        videoUtil.sound(input,output);
    }
}
