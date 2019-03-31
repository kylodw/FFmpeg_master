package com.example.administrator.ffmpeg_master;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * surface可以设置输出格式
 */
public class CustomSurfaceView extends SurfaceView {
    public CustomSurfaceView(Context context) {
        super(context);
        init();
    }

    public CustomSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public CustomSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }
    //1,加锁 lock window
    //2，缓冲区赋值
    //3,unlock window

    private void init(){
        SurfaceHolder holder=getHolder();
        //设置像素格式
        holder.setFormat(PixelFormat.RGBA_8888);
    }
}
