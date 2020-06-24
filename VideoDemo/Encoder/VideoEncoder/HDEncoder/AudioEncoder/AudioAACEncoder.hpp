//
//  AudioAACEncoder.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/12.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef AudioAACEncoder_hpp
#define AudioAACEncoder_hpp
#include "EncoderHeader.h"

#include <stdio.h>

class AudioAACEncoder {
private:
    AVFormatContext     *avFormatContext;
    AVCodecContext      *avCodecContext;
    AVCodec             *audio_codec;
    AVStream            *audioStream;
    AVFrame             *input_frame;
    AVFrame             *swrFrame;
    SwrContext          *swrContext;
    AVPacket            pkt;
    
    int     got_output;
    bool    isWriteHeaderSuccess;
    double  duration;
    int     buffer_size;
    uint8_t *samples;
    int     samplesCursor;
    uint8_t **convert_data;
    uint8_t *swrBuffer;
    int     swrBufferSize;
    
    int     publishBitRate; // 基本的算法是：【码率】(kbps)=【视频大小 - 音频大小】(bit位) /【时间】(秒)
    int     audioChannels;
    int     audioSampleRate;
    
    int     totalSWRTimeMills;
    int     totalEncoderTimeMills;
    int     audio_pts;
    // 初始化
    int alloc_avframe();
    int alloc_audio_stream();
    // 当够了一个frame之后就要编码一个packet
    void encodePacket();
    
public:
    AudioAACEncoder();
    virtual ~AudioAACEncoder();
    
    /**
     * 初始化编码器
     * sample_rate  音频采样率
     * channels     音频声道数
     * bit_rate     音频采集频率
     * return  bool 是否初始化成功
     */
    bool initAACEncoder(int sample_rate,int channels,int bit_rate,int bitsPerSample,const char *aacFilePath);
    bool initAACEncoder(int sample_rate,int channels,int bit_rate,const char *aacFilePath);
    
    /**
     * 开始编码
     */
    void aacEncode(byte *buffer, int size);
    
    /**
     * 释放编码器
     */
    void freeAACEncoder(void);
};

#endif /* AudioAACEncoder_hpp */
