package com.example.administrator.ffmpeg_master.gles;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import com.example.administrator.ffmpeg_master.R;

public class MyGLSurfaceActivity extends AppCompatActivity {
    private MyGLSurfaceView myGLSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        myGLSurfaceView = new MyGLSurfaceView(this);
        setContentView(myGLSurfaceView);
    }
}
