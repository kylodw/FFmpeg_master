package com.example.administrator.ffmpeg_master.live;

import android.hardware.Camera;
import android.view.SurfaceHolder;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/02
 */
public class LivePusher implements SurfaceHolder.Callback {
    private SurfaceHolder surfaceHolder;
    VideoPusher videoPusher;
    AudioPusher audioPusher;
    LiveUtil liveUtil;
    public LivePusher(SurfaceHolder holder) {
        this.surfaceHolder = holder;
        surfaceHolder.addCallback(this);
        livePrepare();

    }

    private void livePrepare() {
         liveUtil = new LiveUtil();

        VideoParams videoParams = new VideoParams(480, 320, Camera.CameraInfo.CAMERA_FACING_BACK);
        videoPusher = new VideoPusher(surfaceHolder, videoParams,liveUtil);
        AudioParams audioParams = new AudioParams();
        audioPusher = new AudioPusher(audioParams,liveUtil);


    }

    public void switchCamera() {
        videoPusher.switchCamera();
    }

    public void startPush(String url) {
        audioPusher.startPush();
        videoPusher.startPush();
        liveUtil.startPush(url);

    }

    public void stopPush() {
        videoPusher.stopPush();
        audioPusher.stopPush();
        liveUtil.stopPush();
    }

    public void relase() {
        videoPusher.destroy();
        audioPusher.destroy();
        liveUtil.release();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        stopPush();
        relase();
    }
}
