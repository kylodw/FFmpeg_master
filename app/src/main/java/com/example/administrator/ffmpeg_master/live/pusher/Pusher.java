package com.example.administrator.ffmpeg_master.live.pusher;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/02
 */
public abstract class Pusher {
    abstract void startPush();
    abstract void stopPush();
    abstract void destroy();
}
