package com.example.administrator.ffmpeg_master;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraCharacteristics.Key;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.core.app.ActivityCompat;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.util.Size;
import android.util.SparseIntArray;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class Camera2Activity extends AppCompatActivity {
    private SurfaceView surfaceview;
    private static final SparseIntArray ORIENTATIONS = new SparseIntArray();
    public static final String TAG = "Camera2Activity";

    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }

    private SurfaceHolder surfaceHolder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera2);
        surfaceview = findViewById(R.id.textureView);
        surfaceHolder = surfaceview.getHolder();
        surfaceHolder.setKeepScreenOn(true);
        surfaceHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                initCamera2();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
    }

    String mCameraID;
    private ImageReader imageReader;
    private CameraManager cameraManager;
    private CameraDevice cameraDevice;
    private Handler mainHandler;
    private Handler childHandler;

    private void initCamera2() {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            cameraManager = (CameraManager) getSystemService(CAMERA_SERVICE);
            HandlerThread handlerThread = new HandlerThread("Camera2");
            handlerThread.start();
            mCameraID = "" + CameraCharacteristics.LENS_FACING_FRONT;
            try {
                CameraCharacteristics cameraCharacteristics = cameraManager.getCameraCharacteristics(mCameraID);
                StreamConfigurationMap configurationMap = cameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                Size[] sizes = configurationMap.getOutputSizes(SurfaceHolder.class);
                Log.e(TAG, "size[0]:" + sizes[0] + "    size[1]:" + sizes[1]);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
            mainHandler = new Handler(getMainLooper());
            imageReader = ImageReader.newInstance(1080, 1920, ImageFormat.YUV_420_888, 1);
            childHandler = new Handler(handlerThread.getLooper());
            imageReader.setOnImageAvailableListener(new ImageReader.OnImageAvailableListener() {
                @Override
                public void onImageAvailable(ImageReader reader) {
                    //照片数据可用时
                    Image image = reader.acquireLatestImage();
                    //将这帧数据转成字节数组，类似于Camera1的PreviewCallback回调的预览帧数据
                    ByteBuffer byteBuffer = image.getPlanes()[0].getBuffer();
                    byte[] data = new byte[byteBuffer.remaining()];
                    byteBuffer.get(data);
                    Log.e("onImageAvailable", "onImageAvailable: data size" + data.length);
                    image.close();
                }
            }, mainHandler);

            try {
                if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                    return;
                }
                cameraManager.openCamera(mCameraID, new CustomStat(), mainHandler);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }

        }


    }

    private boolean isPushing = false;

    public void startPre(View view) {
        isPushing = true;
    }

    public void endPre(View view) {
        isPushing = false;
        if (cameraCaptureSession != null) {
            try {
                cameraCaptureSession.stopRepeating();
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }
    }

    public void pict(View view) {
        takePicture();
    }

    private class CustomStat extends CameraDevice.StateCallback {

        @Override
        public void onOpened(@androidx.annotation.NonNull CameraDevice camera) {
            cameraDevice = camera;
            takePreview();
        }

        @Override
        public void onDisconnected(@androidx.annotation.NonNull CameraDevice camera) {
            if (null != cameraDevice) {
                cameraDevice.close();
                cameraDevice = null;
            }
        }

        @Override
        public void onError(@androidx.annotation.NonNull CameraDevice camera, int error) {

        }
    }

    private CameraCaptureSession cameraCaptureSession;

    private void takePreview() {
        try {
            final CaptureRequest.Builder builder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            builder.addTarget(surfaceHolder.getSurface());
            cameraDevice.createCaptureSession(Arrays.asList(surfaceHolder.getSurface(), imageReader.getSurface()), new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(@androidx.annotation.NonNull CameraCaptureSession session) {
                    cameraCaptureSession = session;
                    builder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                    builder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);
                    builder.addTarget(imageReader.getSurface());
                    CaptureRequest captureRequest = builder.build();
                    try {

                        cameraCaptureSession.setRepeatingRequest(captureRequest, null, childHandler);

                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed(@androidx.annotation.NonNull CameraCaptureSession session) {

                }
            }, childHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void takePicture() {

        if (cameraDevice == null) {
            return;
        }
        // 创建拍照需要的CaptureRequest.Builder
        final CaptureRequest.Builder captureRequestBuilder;
        try {
            captureRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            // 将imageReader的surface作为CaptureRequest.Builder的目标
            captureRequestBuilder.addTarget(imageReader.getSurface());
            // 自动对焦
            captureRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
//            // 自动曝光
//            captureRequestBuilder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);
            // 获取手机方向
            int rotation = getWindowManager().getDefaultDisplay().getRotation();
            // 根据设备方向计算设置照片的方向
            captureRequestBuilder.set(CaptureRequest.JPEG_ORIENTATION, ORIENTATIONS.get(rotation));
            // 拍照
            CaptureRequest mCaptureRequest = captureRequestBuilder.build();
            //CameraCaptureSession调用captuer开始拍照，调用setRepeatingRequest进行预览
            cameraCaptureSession.capture(mCaptureRequest, null, childHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

    }


}
