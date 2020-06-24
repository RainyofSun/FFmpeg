//
//  VideoH264SoftEncoder.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/9.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "VideoH264SoftEncoder.hpp"

// 编码初始化
bool VideoH264SoftEncoder::initX264Encoder(const char *h264path, int width, int height, int videoBitRate, int frameRate) {
    avcodec_register_all();
    
    frameCounter    = 0;
    frameWidth      = width;
    frameHeight     = height;
    
    pCodecCtx = avcodec_alloc_context3(pCodec);
    pCodecCtx->codec_id = AV_CODEC_ID_H264;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P; // 可能有问题
    pCodecCtx->width = frameWidth;
    pCodecCtx->height = frameHeight;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = frameRate;
    pCodecCtx->bit_rate = videoBitRate;
    pCodecCtx->gop_size = 60;
    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;
    
    AVDictionary *params = NULL;
    if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        //编码速度值有ultrafast、superfast、veryfast、faster、fast、medium、slow、slower、veryslow、placebo，越快视频质量则越差
        av_dict_set(&params, "preset", "slow", 0);
        av_dict_set(&params, "tune", "zerolatency", 0);
    }
    
    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec) {
        printf("未能找到编码器");
        return false;
    }
    if (avcodec_open2(pCodecCtx, pCodec, &params)) {
        printf("打开编码器失败");
        return false;
    }
    
    pFrame = av_frame_alloc();
    pFrame->width = frameWidth;
    pFrame->height = frameHeight;
    pFrame->format = AV_PIX_FMT_YUV420P;
    
    avpicture_fill((AVPicture *)pFrame, NULL, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    
    pictureSize = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    av_new_packet(&packet, pictureSize);
    
    return true;
}

// 开始编码
void VideoH264SoftEncoder::encode(uint8_t *buffer) {
    // YUV data
    pFrame->data[0] = buffer;                                           // Y
    pFrame->data[1] = pFrame->data[0] + frameWidth * frameHeight;       // U
    pFrame->data[2] = pFrame->data[1] + (frameWidth * frameHeight)/4;   // V
    
    // PTS
    pFrame->pts = frameCounter;
    
    // Encode
    int grop_picture = 0;
    int ret = avcodec_encode_video2(pCodecCtx, &packet, pFrame, &grop_picture);
    if (ret < 0) {
        printf("编码失败");
    }
    if (grop_picture == 1) {
        printf("Successed to encode frame: %5d\tsize:%5d\n",frameCounter,packet.size);
        frameCounter ++;
        
        // 保存h264视频流
        av_free_packet(&packet);
    }
}

// 销毁编码器
void VideoH264SoftEncoder::destroyEncoder() {
    avcodec_close(pCodecCtx);
    av_free(pFrame);
    pCodecCtx = NULL;
    pFrame = NULL;
    printf("Codec Dealloc");
}
