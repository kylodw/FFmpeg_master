package com.example.administrator.ffmpeg_master.video;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import com.example.administrator.ffmpeg_master.R;

public class VideoActivity extends AppCompatActivity {
    private JfPlayer jfPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);
        jfPlayer = new JfPlayer();
        jfPlayer.setJfOnPreparedListener(new JfOnPreparedListener() {
            @Override
            public void onPrepared() {
                jfPlayer.start();
            }
        });
    }

    public void beginAudio(View view) {
        jfPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        jfPlayer.prepared();
    }
}
