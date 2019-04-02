package com.example.administrator.ffmpeg_master.live;

import android.hardware.Camera;
import android.hardware.camera2.CameraManager;
import android.util.Log;
import android.view.SurfaceHolder;

import java.io.IOException;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/02
 */
public class VideoPusher extends Pusher implements SurfaceHolder.Callback, Camera.PreviewCallback {
    private final SurfaceHolder holder;
    private Camera mCamera;
    CameraManager cameraManager;
    private VideoParams videoParams;
    private byte[] buffer;
    private boolean isPushing = false;
    private LiveUtil liveUtil;

    public VideoPusher(SurfaceHolder surfaceHolder, VideoParams videoParams, LiveUtil liveUtil) {
        this.holder = surfaceHolder;
        this.videoParams = videoParams;
        this.liveUtil = liveUtil;
        holder.addCallback(this);
        buffer = new byte[3110400];
    }

    @Override
    void startPush() {
        isPushing = true;
    }

    @Override
    void stopPush() {
        isPushing = false;
    }

    @Override
    void destroy() {
        stopPreview();
    }

    private void startPreview() {
        try {
            mCamera = Camera.open(videoParams.getCameraId());
            mCamera.setPreviewDisplay(holder);

            mCamera.addCallbackBuffer(buffer);
            mCamera.setPreviewCallbackWithBuffer(this);
            mCamera.startPreview();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void stopPreview() {
        if (mCamera != null) {
            mCamera.stopPreview();
            mCamera.release();
            mCamera = null;
        }
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        //这里开始预览
        //后置摄像头
        startPreview();

    }


    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopPreview();
    }


    public void switchCamera() {
        if (videoParams.getCameraId() == Camera.CameraInfo.CAMERA_FACING_BACK) {
            videoParams.setCameraId(Camera.CameraInfo.CAMERA_FACING_FRONT);
        } else {
            videoParams.setCameraId(Camera.CameraInfo.CAMERA_FACING_BACK);
        }
        stopPreview();
        startPreview();
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        if (mCamera != null) {
            mCamera.addCallbackBuffer(buffer);
        }
        if (isPushing) {
            liveUtil.sendVideo(data);
            //获取图像数据,不断的被调用,回调给native层
            Log.e("onPreviewFrame", "图像采集");
        }
    }
}
