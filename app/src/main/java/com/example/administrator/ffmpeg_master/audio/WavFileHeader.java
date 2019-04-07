package com.example.administrator.ffmpeg_master.audio;

/**
 * @Author kylodw
 * @Description:
 * @Date 2019/04/07
 */
public class WavFileHeader {
    /**
     * 通过“ChunkID”来表示这是一个 “RIFF”格式的文件
     */
    public String mChunkID = "RIFF";
    /**
     * “ChunkSize”则记录了整个 wav 文件的字节数
     */
    public int mChunkSize = 0;
    /**
     * 通过“Format”填入“WAVE”来标识这是一个 wav 文件
     */
    public String mFormat = "WAVE";

 //------------------------------------第一部分结束(属于最“顶层”的信息块)---------------------

 //-------------------------------fmt信息块 主要记录了本 wav 音频文件的详细音频参数信息，例如：通道数、采样率、位宽等等---------------------------

    public String mSubChunk1ID = "fmt ";
    public int mSubChunk1Size = 16;
    public short mAudioFormat = 1;
    public short mNumChannel = 1;
    public int mSampleRate = 8000;
    public int mByteRate = 0;
    public short mBlockAlign = 0;
    public short mBitsPerSample = 8;

 //-----------------------------------第三部分，属于“data”信息块，由“Subchunk2Size”这个字段来记录后面存储的二进制原始音频数据的长度。-----------------

    public String mSubChunk2ID = "data";
    public int mSubChunk2Size  = 0;

    public WavFileHeader() {

    }

    public WavFileHeader(int sampleRateInHz, int bitsPerSample, int channels) {
        mSampleRate = sampleRateInHz;
        mBitsPerSample = (short)bitsPerSample;
        mNumChannel = (short)channels;
        mByteRate = mSampleRate*mNumChannel*mBitsPerSample/8;
        mBlockAlign = (short)(mNumChannel*mBitsPerSample/8);
    }
}
