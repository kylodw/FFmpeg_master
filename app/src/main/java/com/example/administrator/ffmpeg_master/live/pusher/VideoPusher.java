package com.example.administrator.ffmpeg_master.live.pusher;

import android.app.Activity;
import android.content.Context;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.camera2.CameraManager;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.example.administrator.ffmpeg_master.live.LiveUtil;
import com.example.administrator.ffmpeg_master.live.params.VideoParams;

import java.io.IOException;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/02
 */
public class VideoPusher extends Pusher implements SurfaceHolder.Callback, Camera.PreviewCallback {
    private final SurfaceHolder holder;
    private final Context context;
    private Camera mCamera;
    CameraManager cameraManager;
    private VideoParams videoParams;
    private byte[] buffer;
    private boolean isPushing = false;
    private LiveUtil liveUtil;

    public VideoPusher(SurfaceHolder surfaceHolder, VideoParams videoParams, LiveUtil liveUtil, Context context) {
        this.holder = surfaceHolder;
        this.videoParams = videoParams;
        this.liveUtil = liveUtil;
        this.context = context;
        holder.addCallback(this);
        buffer = new byte[3110400];
    }

    @Override
    void startPush() {
        liveUtil.setVideoOptions(videoParams.getWidth(), videoParams.getHeight(), videoParams.getBitrate(), videoParams.getFps());
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

            Camera.Parameters parameters = mCamera.getParameters();
            //像素格式
            parameters.setPreviewFormat(ImageFormat.NV21);
            //宽高
            parameters.setPreviewSize(videoParams.getWidth(), videoParams.getHeight());
//            parameters.setPreviewFpsRange(videoParams.getFps() - 1, videoParams.getFps());
            mCamera.setParameters(parameters);
            setCameraDisplayOrientation((Activity) context, videoParams.getCameraId(), mCamera);
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

        }
    }

    private static void setCameraDisplayOrientation(Activity activity, int cameraId, Camera camera) {
        Camera.CameraInfo info =
                new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);
        int rotation = activity.getWindowManager().getDefaultDisplay()
                .getRotation();
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                break;
        }

        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;  // compensate the mirror
        } else {  // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        camera.setDisplayOrientation(result);
    }
}
