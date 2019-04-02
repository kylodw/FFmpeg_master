package com.example.administrator.ffmpeg_master.live;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/02
 */
public class VideoParams {
    private int width;
    private int height;
    private int cameraId;

    public VideoParams(int width, int height, int cameraId) {
        this.width = width;
        this.height = height;
        this.cameraId = cameraId;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public int getCameraId() {
        return cameraId;
    }

    public void setCameraId(int cameraId) {
        this.cameraId = cameraId;
    }
}
