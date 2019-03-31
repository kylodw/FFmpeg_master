package com.example.administrator.ffmpeg_master;

import android.content.Intent;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(impleStringFromJNI());

//        decode(folderurl + "/" + "test.mp4", folderurl + "/" + "output.yuv");
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native String impleStringFromJNI();

    public native int decode(String inputUrl, String outputUrl);


    public native int stream(String input, String output);   //推流


    public native int ffmpegcore(String[] argv);

    public void decodeClick(View view) {
        String folderurl = Environment.getExternalStorageDirectory().getPath();
        String input = folderurl + "/123.mp4";
        String output = folderurl + "/output.yuv";
        File file=new File(input);
        if (!file.exists()){
            Toast.makeText(this, "文件不存在"+input, Toast.LENGTH_SHORT).show();
            return;
        }
        decode(input, output);
        Log.e("Main", folderurl);
    }

    public void streamClick(View view) {
        Toast.makeText(this, "推流得好好整理", Toast.LENGTH_SHORT).show();

//        String folderurl = Environment.getExternalStorageDirectory().getPath();
//        String input = folderurl + "/test.mp4";
//        String output = "rtmp://192.168.31.126:1935/live/livestream";
//        decode(input, output);
//        Log.e("Main", folderurl);
    }

    public void commandClick(View view) {
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

    public void video_util_click(View view) {

    }

    public void gotoSurface(View view) {
        Intent it=new Intent(this,SurfaceActivity.class);
        startActivity(it);
    }

    public void gotoAudio(View view) {
        Intent it=new Intent(this,AudioActivity.class);
        startActivity(it);
    }
}
