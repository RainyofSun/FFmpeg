//
//  LRVideoAudioMuxer.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/22.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef LRVideoAudioMuxer_hpp
#define LRVideoAudioMuxer_hpp

#include <stdio.h>
#include "LRAVPacketList.hpp"
#include "EncoderHeader.h"

class LRVideoAudioMuxer {
private:
    AVOutputFormat      *ofmt;
    AVFormatContext     *ofmt_ctx;
    AVBSFContext        *h264Ctx;
    AVBSFContext        *aacCtx;
    const AVBitStreamFilter   *h264bsfc;
    const AVBitStreamFilter   *aacbsfc;
    
    /* transfer a/v packet data */
    LRAVPacketList      m_videoListPacket;
    LRAVPacketList      m_audioListPacket;
    
    /* current capture a/v stream */
    AVStream            *m_video_stream;
    AVStream            *m_audio_stream;

    /** time base */
    AVRational          in_v_stream_time;
    AVRational          in_a_stream_time;
    AVRational          in_v_frame_rate;
    AVRational          in_a_frame_rate;
    
    const char *muxFilePath;
    bool hasFilePath;
    bool writeHeaderSeccess;
    int  frame_index;
    
    pthread_t           m_muxThread;
    pthread_mutex_t     m_muxLock;
    
    void initCodec();
    int initBitStreamFilter();
    void initGlobalVar();
    int configureFFmpegFormat();
    int openOutputFile();
    static void *MuxAVPacket(void *arg);
    void dispatchAVData();
    void productAVPacket(AVPacket *mux_packet);
    
public:
    /**
     * 准备混流
     */
    bool prepareForMux(const char *muxFilePath);
    
    /**
     * 初始化视频/音频BitStreamFilter
     */
    bool initializationMuxBitStreamFilter(AVStream *video_stream,AVStream *audio_stream);
    
    /**
     * 追加视频数据至混流器
     */
    void addVideoData(AVPacket *video_pkt);
    
    /**
     * 追加音频数据至混流器
     */
    void addAudioData(AVPacket *audio_pkt);
    
    /**
     * 释放混流器
     */
    void freeMuxHander();
};

#endif /* LRVideoAudioMuxer_hpp */
