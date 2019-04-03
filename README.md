# FFmpeg_master
学习FFmpeg

POSIX 标准
man pthread create 

> 三个线程   两个队列实现音视频同步

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

### 1,IPB帧 
常见的压缩方式
I帧：关键帧 帧内压缩，包含完整的画面
P帧：差别帧，与上一个关键帧的差别，需要缓存上一帧才能得到完整的画面 ，依赖于I帧，
B帧：双向差别帧，与前后两帧的差别，需要缓存上一帧和下一帧才能得到完整的画面
I帧 ，帧内压缩帧内检测
P帧B帧 帧间压缩


### 2,判断帧的类型
`avFrame->pict_type`判断帧的类型
AVPacket->flags& AVG_PKT_FLAG_KEY


### 3,DTS和PTS
DTS：DEcoding Time Stamp 解码时间戳
PTS：Presentation Time Stamp  显示时间戳
DTS   123 || PTS 132

has_b_frames 存在b帧

### 4时间基
time_base 时间单位
```
    AVStream stream;
    stream.duration*stream.time_base; 真正的时长
    avFrame->pts  //时间戳
```

 
### 5，音视频同步的三种方案：
音频同步视频：
视频同步音频：
标准时间：

### 6，NALU

  - 4字节起始码 
  - header头 1字节
  - payload