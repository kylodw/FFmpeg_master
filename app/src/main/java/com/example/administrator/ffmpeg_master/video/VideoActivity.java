package com.example.administrator.ffmpeg_master.video;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
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
        jfPlayer.setFlags(1);
        jfPlayer.setSource(Environment.getExternalStorageDirectory() + "/huge.mp4");
        jfPlayer.prepared();
    }

    public void stopAudio(View view) {
        jfPlayer.n_stop();
    }

    public void pauseAudio(View view) {
        jfPlayer.n_pause();
    }

    public void resumeAudio(View view) {
        jfPlayer.n_resume();
    }

    public void pcmPlayLocal(View view) {
        jfPlayer.pcmLocal();
    }

    public void pcmPlayStream(View view) {
        jfPlayer.setFlags(2);
        jfPlayer.setSource(Environment.getExternalStorageDirectory() + "/huge.mp4");
        jfPlayer.prepared();
    }

    public void seekAudio(View view) {

//        jfPlayer.seek(200);
    }
}
