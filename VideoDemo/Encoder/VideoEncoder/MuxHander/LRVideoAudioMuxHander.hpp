//
//  LRVideoAudioMuxHander.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef LRVideoAudioMuxHander_hpp
#define LRVideoAudioMuxHander_hpp

#include <stdio.h>
#include "LRAVPacketList.hpp"
#include "EncoderHeader.h"

class LRVideoAudioMuxHander {
private:
    
    /* control av stream context */
    AVFormatContext     *m_outputContext;
    AVOutputFormat      *m_outputFormat;
    
    /* transfer a/v packet data */
    LRAVPacketList      m_videoListPacket;
    LRAVPacketList      m_audioListPacket;
    
    /* current capture a/v stream */
    AVStream            *m_video_stream;
    AVStream            *m_audio_stream;
    
    AVBitStreamFilterContext *h264bsfc;
    AVBitStreamFilterContext *aacbsfc;
    
    /* global var */
    pthread_t           m_muxThread;
    pthread_mutex_t     m_muxLock;
    AVRational          m_baseTime;
    
    bool                is_ready_for_head;
    const               char *muxFilePath;
    bool                hasFilePath;
    bool                writeHeaderSeccess;
    int                 video_index_out;
    int                 audio_index_out;
    int                 frame_index;
    
    /* methods */
    void initCodec();
    void initGlobalVar();
    int configureFFmpegWithFormat();
    int openOutputFile();
    static void *MuxAVPacket(void *arg);
    void dispatchAVData();
    void addVideoPacket(AVPacket *video_packet, u_int64_t time_stamp);
    void addAudioPacket(AVPacket *audio_packet, u_int64_t time_stamp);
    void productAVPacket(AVPacket *mux_packet);
    
public:
    
    /**
     * 准备混流
     */
    bool prepareForMux(const char *muxFilePath);
    
    /**
     * 追加视频数据至混流器
     */
    void addVideoData(AVPacket *video_pkt, AVStream *video_stream);
    
    /**
     * 追加音频数据至混流器
     */
    void addAudioData(AVPacket *audio_pkt, AVStream *audio_stream);
    
    /**
     * 释放混流器
     */
    void freeMuxHander();
};

#endif /* LRVideoAudioMuxHander_hpp */
