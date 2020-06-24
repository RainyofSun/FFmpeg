//
//  LRAudioAACHDEncoder.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRAudioAACHDEncoder.h"
#import "CommonUtil.h"
#include "AudioEncoder.hpp"

@implementation LRAudioAACHDEncoder
{
    AudioEncoder audioEncoder;
    bool    isSucess;
}

- (instancetype)initWithAudioEncoderConfig:(LRImageCameraConfig *)config {
    if (self = [super init]) {
        isSucess = audioEncoder.initAACEncoder(config.sampleRate, config.channelsPerFrame, config.audioBitRate,[config.audioFilePath cStringUsingEncoding:NSUTF8StringEncoding]);
        if (!isSucess) {
            NSLog(@"创建Audio编码器失败");
        }
    }
    return self;
}

- (void)dealloc {
    if (isSucess) {
        audioEncoder.freeAACEncode();
    }
    NSLog(@"DELLOC %@",NSStringFromClass(self.class));
}

- (void)AACAudioEncoderWithBytes:(uint8_t *)audioData dataLength:(int)length {
    audioEncoder.aacEncode(audioData, length,AudioEncdeorCallBack);
}

void *AudioEncdeorCallBack(AVPacket *video_packet) {
    
    NSLog(@"成功的回调了我");
    return NULL;
}

@end
