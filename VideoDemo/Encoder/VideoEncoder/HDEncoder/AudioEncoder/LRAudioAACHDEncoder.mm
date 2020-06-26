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

@interface LRAudioAACHDEncoder ()

@property (nonatomic)AudioEncoder audioEncoder;
@property (nonatomic,assign)bool isSucess;

@end

@implementation LRAudioAACHDEncoder

- (instancetype)initWithAudioEncoderConfig:(LRImageCameraConfig *)config audioDelegate:(nonnull id<AudioEncoderDelegate>)audioDelegate {
    if (self = [super init]) {
        audioEncode = self;
        self.audioDelegate = audioDelegate;
        _isSucess = _audioEncoder.initAACEncoder(config.sampleRate, config.channelsPerFrame, config.audioBitRate,[config.audioFilePath cStringUsingEncoding:NSUTF8StringEncoding]);
        if (!_isSucess) {
            NSLog(@"创建Audio编码器失败");
        }
        if (self.audioDelegate != nil && [self.audioDelegate respondsToSelector:@selector(encodeAudioStream:)]) {
            [self.audioDelegate encodeAudioStream:_audioEncoder.audio_stream];
        }
    }
    return self;
}

- (void)dealloc {
    if (_isSucess) {
        _audioEncoder.freeAACEncode();
    }
    NSLog(@"DELLOC %@",NSStringFromClass(self.class));
}

- (void)AACAudioEncoderWithBytes:(uint8_t *)audioData dataLength:(int)length {
    _audioEncoder.aacEncode(audioData, length,AudioEncdeorCallBack);
}

#pragma mark - private methods
- (void)audioEncdeo:(AVPacket *)audioPacket {
    if (self.audioDelegate != nil && [self.audioDelegate respondsToSelector:@selector(audioEncodecData:)]) {
        [self.audioDelegate audioEncodecData:audioPacket];
    }
}

#pragma mark - C Functions
void *AudioEncdeorCallBack(AVPacket *audio_packet) {
    [audioEncode audioEncdeo:audio_packet];
    return NULL;
}

@end
