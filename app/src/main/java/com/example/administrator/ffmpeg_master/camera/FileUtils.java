package com.example.administrator.ffmpeg_master.camera;

import android.annotation.TargetApi;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.media.Image;
import android.media.MediaCodecInfo;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * 文件处理工具类
 * Created by renhui on 2017/9/25.
 */
public class FileUtils {

    private static final String MAIN_DIR_NAME = "/android_records";
    private static final String BASE_VIDEO = "/video/";
    private static final String BASE_EXT = ".mp4";

    private SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy_MM_dd_HH_mm");
    private String currentFileName = "-";
    private String nextFileName;

    public FileUtils() {
    }

    public boolean requestSwapFile() {
        return requestSwapFile(false);
    }

    public boolean requestSwapFile(boolean force) {
        //SD 卡可读写
        String fileName = getFileName();
        boolean isChanged = false;

        if (!currentFileName.equalsIgnoreCase(fileName)) {
            isChanged = true;
        }

        if (isChanged || force) {
            nextFileName = getSaveFilePath(fileName);
            return true;
        }

        return false;
    }

    public String getNextFileName() {
        return nextFileName;
    }

    private String getFileName() {
        String format = simpleDateFormat.format(System.currentTimeMillis());
        return format;
    }

    private String getSaveFilePath(String fileName) {
        currentFileName = fileName;
        StringBuilder fullPath = new StringBuilder();
        fullPath.append(getExternalStorageDirectory());
        //检查内置卡剩余空间容量,并清理
        checkSpace();
        fullPath.append(MAIN_DIR_NAME);
        fullPath.append(BASE_VIDEO);
        fullPath.append(fileName);
        fullPath.append(BASE_EXT);

        String string = fullPath.toString();
        File file = new File(string);
        File parentFile = file.getParentFile();
        if (!parentFile.exists()) {
            parentFile.mkdirs();
        }

        return string;
    }

    /**
     * 检查剩余空间
     */
    private void checkSpace() {
        StringBuilder fullPath = new StringBuilder();
        String checkPath = getExternalStorageDirectory();
        fullPath.append(checkPath);
        fullPath.append(MAIN_DIR_NAME);
        fullPath.append(BASE_VIDEO);

        if (checkCardSpace(checkPath)) {
            File file = new File(fullPath.toString());

            if (!file.exists()) {
                file.mkdirs();
            }

            String[] fileNames = file.list();
            if (fileNames.length < 1) {
                return;
            }

            List<String> fileNameLists = Arrays.asList(fileNames);
            Collections.sort(fileNameLists);

            for (int i = 0; i < fileNameLists.size() && checkCardSpace(checkPath); i++) {
                //清理视频
                String removeFileName = fileNameLists.get(i);
                File removeFile = new File(file, removeFileName);
                try {
                    removeFile.delete();
                    Log.e("angcyo-->", "删除文件 " + removeFile.getAbsolutePath());
                } catch (Exception e) {
                    e.printStackTrace();
                    Log.e("angcyo-->", "删除文件失败 " + removeFile.getAbsolutePath());
                }
            }
        }
    }

    private boolean checkCardSpace(String filePath) {
        File dir = new File(filePath);
        double totalSpace = dir.getTotalSpace();//总大小
        double freeSpace = dir.getFreeSpace();//剩余大小
        if (freeSpace < totalSpace * 0.2) {
            return true;
        }
        return false;
    }

    /**
     * 获取sdcard路径
     */
    public static String getExternalStorageDirectory() {
        return Environment.getExternalStorageDirectory().getPath();
    }

    @TargetApi(Build.VERSION_CODES.KITKAT)
    private static boolean isImageFormatSupported(Image image) {
        int format = image.getFormat();
        switch (format) {
            case ImageFormat.YUV_420_888:
            case ImageFormat.NV21:
            case ImageFormat.YV12:
                return true;
        }
        return false;
    }
    private static final int COLOR_FormatI420 = 1;
    private static final int COLOR_FormatNV21 = 2;
    //YUV420SemiPlanar 也是YUV420-888格式的
    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public static byte[] getDataFromImage(Image image, int colorFormat) {
        if (colorFormat != COLOR_FormatI420 && colorFormat != COLOR_FormatNV21) {
            throw new IllegalArgumentException("only support COLOR_FormatI420 " + "and COLOR_FormatNV21");
        }
        if (!isImageFormatSupported(image)) {
            throw new RuntimeException("can't convert Image to byte array, format " + image.getFormat());
        }
        Rect crop = image.getCropRect();
        int format = image.getFormat();
        int width = crop.width();
        int height = crop.height();
        Image.Plane[] planes = image.getPlanes();
        byte[] data = new byte[width * height * ImageFormat.getBitsPerPixel(format) / 8];
        byte[] rowData = new byte[planes[0].getRowStride()];
        //每个分量数据写入到byte[]的初始偏移量
        int channelOffset = 0;
        int outputStride = 1;
        for (int i = 0; i < planes.length; i++) {
            switch (i) {
                case 0:
                    channelOffset = 0;
                    outputStride = 1;
                    break;
                case 1:
                    if (colorFormat == COLOR_FormatI420) {
                        channelOffset = width * height;
                        outputStride = 1;
                    } else if (colorFormat == COLOR_FormatNV21) {
                        channelOffset = width * height + 1;
                        outputStride = 2;
                    }
                    break;
                case 2:
                    if (colorFormat == COLOR_FormatI420) {
                        channelOffset = (int) (width * height * 1.25);
                        outputStride = 1;
                    } else if (colorFormat == COLOR_FormatNV21) {
                        channelOffset = width * height;
                        outputStride = 2;
                    }
                    break;
            }
            ByteBuffer buffer = planes[i].getBuffer();
            int rowStride = planes[i].getRowStride();
            int pixelStride = planes[i].getPixelStride();
            Log.e("pixelStride", "pixelStride " + pixelStride);
            Log.e("pixelStride", "rowStride " + rowStride);
            Log.e("pixelStride", "width " + width);
            Log.e("pixelStride", "height " + height);
            Log.e("pixelStride", "buffer size " + buffer.remaining());
            Log.e("pixelStride","------------------------------------------------");
            int shift = (i == 0) ? 0 : 1;
            int w = width >> shift;
            int h = height >> shift;
            buffer.position(rowStride * (crop.top >> shift) + pixelStride * (crop.left >> shift));
            for (int row = 0; row < h; row++) {
                int length;
                if (pixelStride == 1 && outputStride == 1) {
                    length = w;
                    buffer.get(data, channelOffset, length);
                    channelOffset += length;
                } else {
                    length = (w - 1) * pixelStride + 1;
                    buffer.get(rowData, 0, length);
                    for (int col = 0; col < w; col++) {
                        data[channelOffset] = rowData[col * pixelStride];
                        channelOffset += outputStride;
                    }
                }
                if (row < h - 1) {
                    buffer.position(buffer.position() + rowStride - length);
                }
            }
        }
        return data;
    }
}
