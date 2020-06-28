//
//  AudioEncoder.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/17.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef AudioEncoder_hpp
#define AudioEncoder_hpp

#include <stdio.h>
#include "EncoderHeader.h"

class AudioEncoder {
private:
    
    AVFormatContext     *avFormatCtx;
    AVOutputFormat      *avOutputCtx;
    AVCodecContext      *avAudioCtx;
    AVCodec             *audio_codec;
    AVFrame             *audio_frame;
    
    const char*         aacFilePath;
    int                 sample_rate;
    int                 bit_rate;
    int                 channels;
    int                 buffer_size;
    int                 samplesCursor;
    int                 frame_current,i,ret;
    uint8_t             *pcm_samples;
    bool                writeHeaderSeccess;
    bool                isNeedWriteLocal;
    
    int malloc_audio_stream();
    int malloc_audio_pFrame();
    int openOutputFile();
    int find_audio_codec();
    
    int audio_encode(void *(*AudioEncoderCallBack)(AVPacket *audio_packet));
    
    int flush_encoder();
    
public:
    AVStream            *audio_stream;
    
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
    int aacEncode(byte *buffer, int size,void *(*AudioEncoderCallBack)(AVPacket *audio_packet));
    
    /**
     * 释放编码器
     */
    void freeAACEncode(void);
};

#endif /* AudioEncoder_hpp */
