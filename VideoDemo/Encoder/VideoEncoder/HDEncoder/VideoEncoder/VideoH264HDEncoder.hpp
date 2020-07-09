//
//  VideoH264HDEncoder.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/11.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef VideoH264HDEncoder_hpp
#define VideoH264HDEncoder_hpp

#include <stdio.h>
#include "EncoderHeader.h"

class VideoH264HDEncoder {
private:
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame         *pFrame;
    AVOutputFormat  *avOutputCtx;
    AVFormatContext *avFormatCtx;
    
    int             pictureSize;
    int             encode_frame_count;
    int             frameWidth; // 编码的图像宽度
    int             frameHeight; // 编码的图像高度
    const char*     videoFilePath;
    bool            writeHeaderSeccess;
    bool            isNeedWriteLocal;
    int             encode_result;
    int             frame_rate;
    int64_t         encode_time;
    int64_t         start_time;
    
    void initGlobalVar();
    void initializationAVFrame(void);
    int  initializationCodexCtx(int width, int height,int frameRate,int videoBitRate);
    int  initializationFormat();
    int  openOutputFile();
    int flush_encoder();
    
public:
    AVStream        *video_stream;
    
    /**
     * 视频H264编码
     * width        视频宽
     * height       视频高
     * videoBitRate 编码比特率
     * frameRate    视频帧率
     * return   bool 是否初始化成功
     */
    bool initX264Encoder(int width, int height, int videoBitRate, int frameRate,const char *videoFilePath);
    
    /**
     * 开始编码
     */
    void encode(I420Buffer buffer,uint64_t video_time_mills,void *(*VideoEncodeCallBack)(MediaVideoPacket video_packet));
    
    /**
     * 销毁编码器
     */
    void freeEncoder(void);
};

#endif /* VideoH264HDEncoder_hpp */
