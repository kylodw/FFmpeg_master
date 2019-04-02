# FFmpeg_master
学习FFmpeg

POSIX 标准
man pthread create 

>三个线程   两个队列实现音视频同步

两个队列 ：
    - 音频AVPacket Queue
    - 视频AVPacket Queue
三个线程：
    - 生产者：read_stream线程负责不断的读取视频文件中的AVPacket，分别放入两个队列
    - 消费者：
        - 视频解码：从视频AVPacket Queue中获取元素解码绘制
        - 音频解码： 从音频AVPacket Queue中获取元素 解码播放
AVPacket队列   （压缩的数据）
生产效率要高于消费者
