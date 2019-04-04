package com.example.administrator.ffmpeg_master.live.pusher;

import android.content.Context;
import android.hardware.Camera;
import android.view.SurfaceHolder;

import com.example.administrator.ffmpeg_master.live.LiveListener;
import com.example.administrator.ffmpeg_master.live.LiveUtil;
import com.example.administrator.ffmpeg_master.live.params.AudioParams;
import com.example.administrator.ffmpeg_master.live.params.VideoParams;

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
    private Context context;

    public LivePusher(SurfaceHolder holder, Context context) {
        this.surfaceHolder = holder;
        surfaceHolder.addCallback(this);
        this.context = context;
        livePrepare();

    }

    private void livePrepare() {
        liveUtil = new LiveUtil();

        VideoParams videoParams = new VideoParams(480, 320, Camera.CameraInfo.CAMERA_FACING_BACK, 480000, 25);
        videoPusher = new VideoPusher(surfaceHolder, videoParams, liveUtil, context);
        AudioParams audioParams = new AudioParams();
        audioPusher = new AudioPusher(audioParams, liveUtil);


    }

    public void switchCamera() {
        videoPusher.switchCamera();
    }

    public void startPush(String url, LiveListener liveListener) {
        audioPusher.startPush();
        videoPusher.startPush();
        liveUtil.startPush(url);
        liveUtil.setLiveListener(liveListener);

    }

    public void stopPush() {
        videoPusher.stopPush();
        audioPusher.stopPush();
        liveUtil.stopPush();
        liveUtil.removeLiveListener();
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
