package com.example.administrator.ffmpeg_master.gles;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/07
 */
public class MyGLSurfaceView extends GLSurfaceView {
    private MyGLRender myGLRender;

    public MyGLSurfaceView(Context context) {
        super(context);
        setEGLContextClientVersion(2);
        myGLRender = new MyGLRender();
        setRenderer(myGLRender);
    }

    class MyGLRender implements GLSurfaceView.Renderer {

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            GLES20.glViewport(0, 0, width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        }
    }
}
