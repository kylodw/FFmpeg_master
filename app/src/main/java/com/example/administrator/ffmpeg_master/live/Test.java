package com.example.administrator.ffmpeg_master.live;

import android.annotation.TargetApi;
import android.content.Context;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.os.Build;

import androidx.annotation.NonNull;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/02
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class Test extends CameraDevice.StateCallback {
    private CameraManager mCameraManager;
    public Test(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            mCameraManager= (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
//            mCameraManager.openCamera(""+CameraCharacteristics.LENS_FACING_FRONT,this,);

        }

    }


    @Override
    public void onOpened(@NonNull CameraDevice camera) {

    }

    @Override
    public void onDisconnected(@NonNull CameraDevice camera) {

    }

    @Override
    public void onError(@NonNull CameraDevice camera, int error) {

    }
}
