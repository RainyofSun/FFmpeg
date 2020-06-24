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

static LRAudioAACHDEncoder *audioEncode = nil;

@implementation LRAudioAACHDEncoder
{
    AudioEncoder audioEncoder;
    bool    isSucess;
}

- (instancetype)initWithAudioEncoderConfig:(LRImageCameraConfig *)config {
    if (self = [super init]) {
        audioEncode = self;
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

#pragma mark - private methods
- (void)audioEncdeo:(AVPacket *)audioPacket withAudioStream:(AVStream *)audioStream {
    if (self.audioDelegate != nil && [self.audioDelegate respondsToSelector:@selector(audioEncodecData:withAudioStream:)]) {
        [self.audioDelegate audioEncodecData:audioPacket withAudioStream:audioStream];
    }
}

#pragma mark - C Functions
void *AudioEncdeorCallBack(AVPacket *audio_packet,AVStream *audio_stream) {
    [audioEncode audioEncdeo:audio_packet withAudioStream:audio_stream];
    return NULL;
}

@end
