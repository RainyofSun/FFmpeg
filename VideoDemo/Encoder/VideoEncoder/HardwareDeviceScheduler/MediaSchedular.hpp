//
//  MediaSchedular.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/23.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef MediaSchedular_hpp
#define MediaSchedular_hpp

#include <stdio.h>
#include "EncoderHeader.h"
#include "VideoH264HDEncoder.hpp"
#include "AudioEncoder.hpp"
#include "LRVideoAudioMuxer.hpp"
#include "NV12ToYUV420P.hpp"

typedef struct {
    // VIDEO
    /** video_width */
    int video_width;
    /** video_height */
    int video_height;
    /** video_bit_rate */
    int video_bit_rate;
    /** frame_rate */
    int frame_rate;

    // AUDIO
    /** sample_rate */
    int sample_rate;
    /** channels */
    int channels;
    /** audio_bit_rate */
    int audio_bit_rate;
}CaptureParameters;

class MediaSchedular {
private:
    
    VideoH264HDEncoder      video_encoder;
    AudioEncoder            audio_encoder;
    NV12ToYUV420P           pixel_convert;
    LRVideoAudioMuxer       mux_Hander;
    
    const char *save_video_file_path;
    const char *save_audio_file_path;
    const char *save_mux_mp4_file_path;
    bool        create_video_encoder;
    bool        create_audio_encoder;
    bool        create_mux_hander;
    
    // 初始化参数集
    void initGlobalVar();
    // 编码回调
    static void *VideoEncodeCallBack(MediaVideoPacket video_packet);
    static void *AudioEncodeCallBack(MediaAudioPacket audio_packet);

public:
    
    /**
     * 初始化调度器
     */
    int initializationMediaSchedular();
    /**
     * 准备采集
     */
    int prepareCapture();
    
    /**
     * 开始采集
     */
    int startCapture();
    
    /**
     * 结束采集
     */
    int stopCapture();
    
    /**
     * 切换摄像头(仅适用于手机设备) 0/前置 1/后置
     */
    int swicthCamera();
    
    /**
     * 准备编码 视频/h264编码 音频/aac编码
     */
    int prepareEncoder(CaptureParameters parameters);
    
    /**
     * 编码视频
     */
    int videoEncode(uint8_t *y_frame, uint8_t *uv_frame, int src_stride_y, int src_stride_uv, int width, int height);
    
    /**
     * 编码音频
     */
    int audioEncode(uint8_t *audio_data, int data_length);

    /**
     * 释放编码器
     */
    int freeEncoder();
    
    /**
     * 文件存放地址-->分步调试使用
     */
    void captureFilePath(const char *vidoe_file_path,const char *audio_file_path,const char *mp4_file_path);
};

#endif /* MediaSchedular_hpp */
