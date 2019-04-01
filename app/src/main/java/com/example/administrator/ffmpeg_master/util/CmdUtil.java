package com.example.administrator.ffmpeg_master.util;

import android.os.Environment;
import android.util.Log;

import java.io.File;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/01
 */
public class CmdUtil {
    public static void getCmd(){
        File file = new File(Environment.getExternalStorageDirectory(), "sy.mp4");
        File outputFile = new File(Environment.getExternalStorageDirectory(), "test.png");
        File outputFiles = new File(Environment.getExternalStorageDirectory(), "265test.mp4");
        //ffmpeg -y -t 60 -i input.mp4 -i logo1.png -i logo2.png -filter_complex
        // "overlay=x=if(lt(mod(t\,20)\,10)\,10\,NAN ):y=10,overlay=x=if(gt(mod(t\,20)\,10)\,W-w-10\,NAN ) :y=10" output.mp4

//        String[] cmds = {"ffmpeg", "-i", file.getAbsolutePath(), outputFile.getAbsolutePath()};
//        String[] cmds = {"ffmpeg", "-y", "-t", "60", "-i", file.getAbsolutePath(), "-i",
//                "test.png", "-filter_complex", "overlay=x=if(lt(mod(t\\,20)\\,10)\\,10\\,NAN ):y=10,overlay=x=if(gt(mod(t\\,20)\\,10)\\,W-w-10\\,NAN ) :y=10",
//                outputFile.getAbsolutePath()};
        //ffmpeg –i input.flv -acodec copy-vcodec copy -vf 'movie=test.png[watermark];[in][watermark]overlay=10:10:1[out]' output.flv
//        String[] cmds = {"ffmpeg", "-i", file.getAbsolutePath(),
//                "-acodec","copy-vcodec", "copy","-vf",
//                "movie=test.png[watermark];[in][watermark]overlay=10:10:1[out]",
//                outputFile.getAbsolutePath()};
        //ffmpeg -i test.asf -vframes 30 -y -f gif a.gif
//        String[] cmds = {"ffmpeg", "-i", file.getAbsolutePath(), "-vframes", "30", "-y", "-f", "gif", outputFile.getAbsolutePath()};
        //ffmpeg -vcodec mpeg4 -b 1000 -r 10 -g 300 -vd x11:0,0 -s 1024x768 ~/test.avi
        // String[] cmds = {"ffmpeg", "vcodec", "mpeg4", "-b", "1000", "-r", "10", "-g", "300", "-vd", "x11:0,0", "-s", "1024x768", file.getAbsolutePath()};

        //ffmpeg -i test.mp4 -acodec copy -vn output.aac  提取音频
//        String[] cmds = {"ffmpeg", "-i", file.getAbsolutePath(), "-acodec", "copy", "-vn", outputFile.getAbsolutePath()};
        //ffmpeg -i input.mp4 -i iQIYI_logo.png -filter_complex overlay output.mp4   //加logo
        //String[] cmds = {"ffmpeg","-i",file.getAbsolutePath(),"-i",outputFile.getAbsolutePath(),"-filter_complex","overlay",outputFiles.getAbsolutePath()};
        //ffmpeg -i input.mp4 -c:v libx265 -x265-params "profile=high:level=3.0" output.mp4
//        String[] cmds = {"ffmpeg", "-i", file.getAbsolutePath(), "-c:v", "libx265", "-x265-params", "\"profile=high:level=3.0\"", outputFiles.getAbsolutePath()};
//        ffmpegcore(cmds);
        Log.e("Main", "执行完了啊");
    }
}
