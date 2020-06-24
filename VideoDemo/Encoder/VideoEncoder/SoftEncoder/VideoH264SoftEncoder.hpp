//
//  VideoH264SoftEncoder.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/9.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef VideoH264Encoder_hpp
#define VideoH264Encoder_hpp

#include <stdio.h>
#include "EncoderHeader.h"

class VideoH264SoftEncoder {
private:
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVPacket         packet;
    AVFrame         *pFrame;
    int             pictureSize;
    int             frameCounter;
    int             frameWidth; // 编码的图像宽度
    int             frameHeight; // 编码的图像高度
    FILE            *pFile;
public:
    /**
     * 视频H264编码
     * h264path     h264裸数据存储路径
     * width        视频宽
     * height       视频高
     * videoBitRate 编码比特率
     * frameRate    视频帧率
     * return   bool 是否初始化成功
     */
    bool initX264Encoder(const char *h264path,int width, int height, int videoBitRate, int frameRate);

    /**
     * 开始编码
     */
    void encode(uint8_t *buffer);

    /**
     * 销毁编码器
     */
    void destroyEncoder(void);
};

#endif /* VideoH264Encoder_hpp */
