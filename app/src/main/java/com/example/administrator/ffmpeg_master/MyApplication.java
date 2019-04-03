package com.example.administrator.ffmpeg_master;

import android.app.Application;
import android.content.Context;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/03
 */
public class MyApplication extends Application {
    public static Context context;

    @Override
    public void onCreate() {
        super.onCreate();
        context = getApplicationContext();
    }
}
