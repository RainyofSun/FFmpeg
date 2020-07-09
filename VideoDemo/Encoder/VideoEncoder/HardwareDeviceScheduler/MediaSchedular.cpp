//
//  MediaSchedular.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/23.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "MediaSchedular.hpp"

#pragma mark - public methods
// 初始化调度器
int MediaSchedular::initializationMediaSchedular() {
    this->initGlobalVar();
    return 0;
}

// 准备开始采集
int MediaSchedular::prepareCapture() {
    
    return 0;
}

// 开始采集
int MediaSchedular::startCapture() {
    return 0;
}

// 结束采集
int MediaSchedular::stopCapture() {
    return 0;
}

// 切换摄像头
int MediaSchedular::swicthCamera() {
    return 0;
}

// 准备编码
int MediaSchedular::prepareEncoder(CaptureParameters parameters) {
    bool success = false;
    success = this->video_encoder.initX264Encoder(parameters.video_width, parameters.video_height, parameters.video_bit_rate, parameters.frame_rate, this->save_video_file_path);
    if (!success) {
        return -1;
    }
    this->create_video_encoder = true;
    success = this->audio_encoder.initAACEncoder(parameters.sample_rate, parameters.channels, parameters.audio_bit_rate, this->save_audio_file_path);
    if (!success) {
        return -1;
    }
    this->create_audio_encoder = true;
    if (this->video_encoder.video_stream && this->audio_encoder.audio_stream) {
        success = this->mux_Hander.prepareForMux(this->save_mux_mp4_file_path, this->video_encoder.video_stream, this->audio_encoder.audio_stream);
        if (success) {
            this->create_mux_hander = success;
            return 0;
        } else {
            return -1;
        }
    }
    return 0;
}

// 编码视频
int MediaSchedular::videoEncode(uint8_t *y_frame, uint8_t *uv_frame, int src_stride_y, int src_stride_uv, int width, int height) {
    I420Buffer buffer = this->pixel_convert.convertNV12BufferToI420Buffer(y_frame, uv_frame, src_stride_y, src_stride_uv, (int)width, (int)height);
    // TODO:: 时间有问题
    this->video_encoder.encode(buffer,getCurrentTime(), this->VideoEncodeCallBack);
    this->pixel_convert.freeI420Buffer(buffer);
    return 0;
}

// 编码音频
int MediaSchedular::audioEncode(uint8_t *audio_data, int data_length) {
    this->audio_encoder.aacEncode(audio_data, data_length, this->AudioEncodeCallBack);
    return 0;
}

// 释放编码器
int MediaSchedular::freeEncoder() {
    if (this->create_video_encoder) {
        this->video_encoder.freeEncoder();
        printf("FREE VIDEO ENCDOER\n");
    }
    if (this->create_audio_encoder) {
        this->audio_encoder.freeAACEncode();
        printf("FREE AUDIO ENCODER\n");
    }
    if (this->create_mux_hander) {
        this->mux_Hander.freeMuxHander();
        printf("FREE MUX HANDER\n");
    }
    return 0;
}

// 文件存放地址-->分步调试使用
void MediaSchedular::captureFilePath(const char *vidoe_file_path, const char *audio_file_path, const char *mp4_file_path) {
    this->save_video_file_path = vidoe_file_path;
    this->save_audio_file_path = audio_file_path;
    this->save_mux_mp4_file_path = mp4_file_path;
}

#pragma mark - private metods
void MediaSchedular::initGlobalVar() {
    this->create_video_encoder = false;
    this->create_audio_encoder = false;
    this->create_mux_hander    = false;
}

#pragma mark - C Functions
// 视频编码回调
void* MediaSchedular::VideoEncodeCallBack(MediaVideoPacket video_packet) {
    
    return NULL;
}

// 音频编码回调
void* MediaSchedular::AudioEncodeCallBack(MediaAudioPacket audio_packet) {
    
    return NULL;
}
