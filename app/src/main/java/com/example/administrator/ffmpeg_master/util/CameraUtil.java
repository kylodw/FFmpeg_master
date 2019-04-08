package com.example.administrator.ffmpeg_master.util;

import android.content.Context;
import android.os.Environment;
import android.util.Log;
import android.util.Size;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import static android.provider.MediaStore.Files.FileColumns.MEDIA_TYPE_IMAGE;
import static android.provider.MediaStore.Files.FileColumns.MEDIA_TYPE_VIDEO;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/08
 */
public class CameraUtil {
    public static Size getMinPreSize(Size[] sizeMap, int surfaceWidth, int surfaceHeight, int maxHeight) {
        // 得到与传入的宽高比最接近的size
        float reqRatio = ((float) surfaceWidth) / surfaceHeight;
        float curRatio;
        List<Size> sizeList = new ArrayList<>();
        Size retSize = null;
        for (Size size : sizeMap) {
            curRatio = ((float) size.getHeight()) / size.getWidth();
            if (reqRatio == curRatio) {
                sizeList.add(size);
            }
        }

        if (sizeList != null && sizeList.size() != 0) {
            for (int i = sizeList.size() - 1; i >= 0; i--) {
                //取Size宽度大于1000的第一个数,这里我们获取大于maxHeight的第一个数，理论上我们是想获取size.getWidth宽度为1080或者1280那些，因为这样的预览尺寸已经足够了
                if (sizeList.get(i).getWidth() >= maxHeight) {
                    retSize = sizeList.get(i);
                    break;
                }
            }

            //可能没有宽度大于maxHeight的size,则取相同比例中最小的那个size
            if (retSize == null) {
                retSize = sizeList.get(sizeList.size() - 1);
            }

        } else {
            retSize = getCloselyPreSize(sizeMap, surfaceWidth, surfaceHeight);
        }
        return retSize;
    }

    // 通过对比得到与宽高比最接近的尺寸（如果有相同尺寸，优先选择，activity我们已经固定了方向，所以这里无需在做判断
    protected static Size getCloselyPreSize(Size[] sizeMap, int surfaceWidth, int surfaceHeight) {
        int ReqTmpWidth;
        int ReqTmpHeight;
        ReqTmpWidth = surfaceHeight;
        ReqTmpHeight = surfaceWidth;
        //先查找preview中是否存在与surfaceview相同宽高的尺寸
        for (Size size : sizeMap) {
            if ((size.getWidth() == ReqTmpWidth) && (size.getHeight() == ReqTmpHeight)) {
                return size;
            }
        }

        // 得到与传入的宽高比最接近的size
        float reqRatio = ((float) ReqTmpWidth) / ReqTmpHeight;
        float curRatio, deltaRatio;
        float deltaRatioMin = Float.MAX_VALUE;
        Size retSize = null;
        for (Size size : sizeMap) {
            curRatio = ((float) size.getWidth()) / size.getHeight();
            deltaRatio = Math.abs(reqRatio - curRatio);
            if (deltaRatio < deltaRatioMin) {
                deltaRatioMin = deltaRatio;
                retSize = size;
            }
        }
        return retSize;
    }
    public static File getOutputMediaFile(Context context,int mediaType) {
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String fileName = null;
        File storageDir = null;
        if (mediaType == MEDIA_TYPE_IMAGE) {
            fileName = "JPEG_" + timeStamp + "_";
            storageDir = context.getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        } else if (mediaType == MEDIA_TYPE_VIDEO) {
            fileName = "MP4_" + timeStamp + "_";
            storageDir =context.getExternalFilesDir(Environment.DIRECTORY_MOVIES);
        }

        File file = null;
        try {
            file = File.createTempFile(
                    fileName,
                    (mediaType == MEDIA_TYPE_IMAGE) ? ".jpg" : ".h264",
                    storageDir
            );
            Log.e("getOutputMediaFile", "getOutputMediaFile: absolutePath==" + file.getAbsolutePath());
        } catch (IOException e) {
            e.printStackTrace();
        }
        return file;
    }

}
