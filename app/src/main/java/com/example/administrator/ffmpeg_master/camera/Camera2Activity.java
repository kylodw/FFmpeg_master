package com.example.administrator.ffmpeg_master.camera;

import android.Manifest;
import android.annotation.TargetApi;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.example.administrator.ffmpeg_master.R;
import com.example.administrator.ffmpeg_master.util.CameraUtil;

import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import static android.provider.MediaStore.Files.FileColumns.MEDIA_TYPE_VIDEO;

@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class Camera2Activity extends AppCompatActivity implements TextureView.SurfaceTextureListener {
    private TextureView textureView;
    public static final String TAG = "Camera2Activity";
    private HandlerThread handlerThread;
    private int width;
    private int height;
    private String mCameraID;
    private String mCameraIdFront;
    //当前是否是前置摄像头
    private boolean isCameraFront = false;
    private CameraCharacteristics cameraCharacteristics;
    private int mSensorOrientation;
    private Size mPreviewSize;
    private Size mCaptureSize;
    private ImageReader imageReader;
    private CameraManager cameraManager;
    private CameraDevice cameraDevice;
    private Handler mainHandler;

    private CameraCaptureSession cameraCaptureSession;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera2);
        textureView = findViewById(R.id.textureView);
        initTextureView();
    }

    private void initTextureView() {
        handlerThread = new HandlerThread("Camera2");
        handlerThread.start();
        mainHandler = new Handler(handlerThread.getLooper());
        textureView.setSurfaceTextureListener(this);
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        this.width = width;
        this.height = height;
        settingCamera(width, height);
        initCamera(mCameraID);
    }

    private void settingCamera(int width, int height) {
        cameraManager = (CameraManager) getSystemService(CAMERA_SERVICE);
        //0表示后置摄像头,1表示前置摄像头
        try {
            mCameraID = cameraManager.getCameraIdList()[0];
            mCameraIdFront = cameraManager.getCameraIdList()[1];
            if (isCameraFront) {
                cameraCharacteristics = cameraManager.getCameraCharacteristics(mCameraIdFront);
            } else {
                cameraCharacteristics = cameraManager.getCameraCharacteristics(mCameraID);
            }
            CameraUtil.isHardwareSupported(cameraCharacteristics);
            StreamConfigurationMap configurationMap = cameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            mSensorOrientation = cameraCharacteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
            mPreviewSize = CameraUtil.chooseVideoSize(configurationMap.getOutputSizes(SurfaceTexture.class));
            mCaptureSize = Collections.max(Arrays.asList(configurationMap.getOutputSizes(ImageFormat.JPEG)), new Comparator<Size>() {
                @Override
                public int compare(Size lhs, Size rhs) {
                    return Long.signum(lhs.getWidth() * lhs.getHeight() - rhs.getHeight() * rhs.getWidth());
                }
            });
            configureTransform(width, height);
            initImageReader();

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }


    }

    private void initImageReader() {
        imageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(), ImageFormat.YUV_420_888, 1);
        Log.e(TAG, "预览的宽高:" + mPreviewSize.getWidth() + "    " + mPreviewSize.getHeight());
        imageReader.setOnImageAvailableListener(new ImageReader.OnImageAvailableListener() {
            @Override
            public void onImageAvailable(ImageReader reader) {
                Image image = reader.acquireNextImage();
//                //将这帧数据转成字节数组，类似于Camera1的PreviewCallback回调的预览帧数据
//                ByteBuffer byteBuffer = image.getPlanes()[0].getBuffer();
//                byte[] data = new byte[byteBuffer.remaining()];
//                int len = image.getFormat();
//                int b = image.getPlanes().length;
//                Log.e(TAG, "这是什么啊？：" + len + " 多少通道：" + b + "宽高:" + image.getWidth() + "  " + image.getHeight());
//                ByteBuffer bufferY = image.getPlanes()[0].getBuffer();
//                ByteBuffer bufferU = image.getPlanes()[1].getBuffer();
//                ByteBuffer bufferV = image.getPlanes()[2].getBuffer();
//                Log.e(TAG, "bufferY" + bufferY.remaining());
//                Log.e(TAG, "bufferU" + bufferU.remaining());
//                Log.e(TAG, "bufferV" + bufferV.remaining());
//                dumpFile(FileUtils.getDataFromImage(image, 1));
//                MediaMuxerThread.addVideoFrameData(FileUtils.getDataFromImage(image, 1));
//                FileOutputStream fileOutputStream = null;
//                BufferedOutputStream bufferedOutputStream = null;
//                try {
//                    fileOutputStream = new FileOutputStream(new File(Environment.getExternalStorageDirectory(), "output_byte.yuv"));
//                    bufferedOutputStream = new BufferedOutputStream(fileOutputStream);
//                    bufferedOutputStream.write(FileUtils.getDataFromImage(image, 1));
//                    bufferedOutputStream.flush();
//                } catch (FileNotFoundException e) {
//                    e.printStackTrace();
//                } catch (IOException e) {
//                    e.printStackTrace();
//                } finally {
//                    try {
//                        bufferedOutputStream.close();
//                    } catch (IOException e) {
//                        e.printStackTrace();
//                    }
//                    try {
//                        fileOutputStream.close();
//                    } catch (IOException e) {
//                        e.printStackTrace();
//                    }
//                }
//                image.close();
                byte[] yuv420buf = camera2ImageToI420(image);
                MediaMuxerThread.addVideoFrameData(yuv420buf);

//                //照片数据可用时
//                Image image = reader.acquireNextImage();
//                Image.Plane[] planes = image.getPlanes();
//                byte[] dataYUV = null;
//                if (planes.length >= 3) {
//                    ByteBuffer bufferY = planes[0].getBuffer();
//                    ByteBuffer bufferU = planes[1].getBuffer();
//                    ByteBuffer bufferV = planes[2].getBuffer();
//                    int lengthY = bufferY.remaining();
//                    int lengthU = bufferU.remaining();
//                    int lengthV = bufferV.remaining();
//
//                    //plane[0]+plane[1]  可以得到NV12
//                    //plane[0]+plane[2]  可以得到NV21
//                    dataYUV = new byte[lengthY + lengthV];
//
////                    dataYUV = new byte[lengthY + lengthU + lengthV];
//                    bufferY.get(dataYUV, 0, lengthY);
//                    bufferV.get(dataYUV, lengthY, lengthV);
////                    bufferU.get(dataYUV, lengthY, lengthU);
////                    bufferV.get(dataYUV, lengthY + lengthU, lengthV);
//                    Log.e("onImageAvailable", "onImageAvailable: data size" + dataYUV.length);
//                    MediaMuxerThread.addVideoFrameData(dataYUV);
//                }
                image.close();


            }
        }, mainHandler);
    }



    private byte[] camera2ImageToI420(Image image) {
        int width = image.getWidth();
        int height = image.getHeight();
        Image.Plane[] planes = image.getPlanes();

        //Y-buffer
        ByteBuffer yBuffer = planes[0].getBuffer();
        int ySize = yBuffer.remaining();
        byte[] yBytes = new byte[ySize];
        yBuffer.get(yBytes);

        //u-buffer
        ByteBuffer uBuffer = planes[1].getBuffer();
        int uSize = uBuffer.remaining();
        byte[] uBytes = new byte[uSize];
        uBuffer.get(uBytes);


        //v-buffer
        ByteBuffer vBuffer = planes[2].getBuffer();
        int vSize = vBuffer.remaining();
        byte[] vBytes = new byte[vSize];
        vBuffer.get(vBytes);


        byte[] ret = new byte[width * height * 3 / 2];
        int yLength = yBytes.length;
        int uLength = uBytes.length + 1;
        int vLength = vBytes.length + 1;


        ByteBuffer bufferY = ByteBuffer.wrap(ret, 0, yLength);
        ByteBuffer bufferU = ByteBuffer.wrap(ret, yLength, uLength / 2);
        ByteBuffer bufferV = ByteBuffer.wrap(ret, yLength + uLength / 2, vLength / 2);

        bufferY.put(yBytes, 0, yLength);
        for (int i = 0; i < uLength; i += 2) {
            bufferU.put(uBytes[i]);
        }
        for (int i = 0; i < vLength; i += 2) {
            bufferV.put(vBytes[i]);
        }


        return ret;
    }

    private void dumpFile(byte[] dataFromImage) {
        FileOutputStream fileOutputStream = null;
        try {
            fileOutputStream = new FileOutputStream(new File(Environment.getExternalStorageDirectory(), "dump_file.yuv"));
            fileOutputStream.write(dataFromImage);
            fileOutputStream.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }


    @TargetApi(Build.VERSION_CODES.M)
    private void initCamera(String mCameraID) {

        try {
            if (checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            cameraManager.openCamera(mCameraID, new CustomStat(), null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private AvcEncoder mAvcEncoder;

    // 屏幕方向发生改变时调用转换数据方法
    private void configureTransform(int viewWidth, int viewHeight) {
        if (null == textureView || null == mPreviewSize) {
            return;
        }
        int rotation = getWindowManager().getDefaultDisplay().getRotation();
        Matrix matrix = new Matrix();
        RectF viewRect = new RectF(0, 0, viewWidth, viewHeight);
        RectF bufferRect = new RectF(0, 0, mPreviewSize.getHeight(), mPreviewSize.getWidth());
        float centerX = viewRect.centerX();
        float centerY = viewRect.centerY();
        if (Surface.ROTATION_90 == rotation || Surface.ROTATION_270 == rotation) {
            bufferRect.offset(centerX - bufferRect.centerX(), centerY - bufferRect.centerY());
            matrix.setRectToRect(viewRect, bufferRect, Matrix.ScaleToFit.FILL);
            float scale = Math.max(
                    (float) viewHeight / mPreviewSize.getHeight(),
                    (float) viewWidth / mPreviewSize.getWidth());
            matrix.postScale(scale, scale, centerX, centerY);
            matrix.postRotate(90 * (rotation - 2), centerX, centerY);
        }
        textureView.setTransform(matrix);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {

    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {

    }


    public void startPre(View view) {

    }

    public void endPre(View view) {
        mAvcEncoder.stopThread();
    }

    public void pict(View view) {
    }

    private boolean isPushing = false;

    public void MuxerClick(View view) {
        Button button = (Button) view;
        if (button.getText().toString().equals("停止")) {
            button.setText("开始");
            MediaMuxerThread.stopMuxer();
            isPushing = false;
        } else {
            button.setText("停止");
            MediaMuxerThread.startMuxer(mPreviewSize.getWidth(),mPreviewSize.getHeight());
            isPushing = true;
        }
    }


    private class CustomStat extends CameraDevice.StateCallback {

        @Override
        public void onOpened(@androidx.annotation.NonNull CameraDevice camera) {
            cameraDevice = camera;
            takePreview();
            if (null != textureView) {
                configureTransform(textureView.getWidth(), textureView.getHeight());
            }
        }

        @Override
        public void onDisconnected(@androidx.annotation.NonNull CameraDevice camera) {
            Toast.makeText(Camera2Activity.this, "断开连接", Toast.LENGTH_SHORT).show();
            if (null != cameraDevice) {
                cameraDevice.close();
                cameraDevice = null;
            }
        }

        @Override
        public void onError(@androidx.annotation.NonNull CameraDevice camera, int error) {
            Toast.makeText(Camera2Activity.this, "打开失败", Toast.LENGTH_SHORT).show();
            if (null != cameraDevice) {
                cameraDevice.close();
                cameraDevice = null;
            }
        }
    }

    private CaptureRequest captureRequest;
    CaptureRequest.Builder builder;

    private void takePreview() {
        if (null == cameraDevice || !textureView.isAvailable() || null == mPreviewSize) {
            return;
        }
        //获取TextureView的SurfaceTexture，作为预览输出载体
        SurfaceTexture mSurfaceTexture = textureView.getSurfaceTexture();
        try {
            //设置TextureView的缓冲区大小
            mSurfaceTexture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());

            builder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            Surface surface = new Surface(mSurfaceTexture);
            builder.addTarget(surface);
            builder.addTarget(imageReader.getSurface());
            //默认预览不开启闪光灯
            builder.set(CaptureRequest.FLASH_MODE, CaptureRequest.FLASH_MODE_OFF);
            // 自动对焦
            builder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
//            builder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            cameraDevice.createCaptureSession(Arrays.asList(surface, imageReader.getSurface()), new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(@androidx.annotation.NonNull CameraCaptureSession session) {
                    cameraCaptureSession = session;
                    builder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
                    try {
                        captureRequest = builder.build();
                        cameraCaptureSession.setRepeatingRequest(captureRequest, null, mainHandler);
                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed(@androidx.annotation.NonNull CameraCaptureSession session) {
                    Toast.makeText(Camera2Activity.this, "配置失败", Toast.LENGTH_SHORT).show();

                }
            }, mainHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }


    @Override
    protected void onResume() {
        isCameraFront = false;
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

}
