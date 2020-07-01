//
//  LRVideoH264HDEncoder.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/11.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRVideoH264HDEncoder.h"
#include "VideoH264HDEncoder.hpp"
#include "NV12ToYUV420P.hpp"
#include "AudioAACEncoder.hpp"

static LRVideoH264HDEncoder *encoder = nil;

@interface LRVideoH264HDEncoder ()

@property (nonatomic)VideoH264HDEncoder videoEncoder;
@property (nonatomic)NV12ToYUV420P vonvert;
@property (nonatomic,assign)bool isSucess;

@end

@implementation LRVideoH264HDEncoder

- (instancetype)initWithVideoEncoderConfig:(LRImageCameraConfig *)config videoDelegate:(nonnull id<VideoEncoderDelegate>)videoDelegate {
    if (self = [super init]) {
        self.videoDelegate = videoDelegate;
        encoder = self;
        _isSucess = _videoEncoder.initX264Encoder(config.videoWidth, config.videoHeight, config.videoBitRate, config.frameRate,[config.videoFilePath cStringUsingEncoding:NSUTF8StringEncoding]);
        if (!_isSucess) {
            NSLog(@"创建Video编码器失败");
        }
    }
    return self;
}

- (void)dealloc {
    NSLog(@"DEALLOC %@",NSStringFromClass(self.class));
}

- (void)H264VideoEncoderWithSampleBuffer:(CMSampleBufferRef)sampleBuffer {
    if (!_isSucess) {
        return;
    }
    CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    CFRetain(pixelBuffer);
    size_t pixelWidth = CVPixelBufferGetWidth(pixelBuffer);
    size_t pixelHeight = CVPixelBufferGetHeight(pixelBuffer);
    
    uint8_t *y_frame = (uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
    uint8_t *uv_frame = (uint8_t *)CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1);
    int src_stride_y = (int)CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0);
    int src_stride_uv = (int)CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 1);
    I420Buffer buffer = _vonvert.convertNV12BufferToI420Buffer(y_frame, uv_frame, src_stride_y, src_stride_uv, (int)pixelWidth, (int)pixelHeight);
    _videoEncoder.encode(buffer,VideoEncdeorCallBack);
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    CFRelease(pixelBuffer);
    _vonvert.freeI420Buffer(buffer);
}

- (void)AACAudioEncoderWithSampleBuffer:(CMSampleBufferRef)sampleBuffer {
    if (!_isSucess) {
        return;
    }
    CMAudioFormatDescriptionRef audioFormatDes =  (CMAudioFormatDescriptionRef)CMSampleBufferGetFormatDescription(sampleBuffer);
    CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
    NSInteger audioSize = CMSampleBufferGetTotalSampleSize(sampleBuffer);
    CFRetain(blockBuffer);
    uint8_t *audioData = (uint8_t *)malloc((size_t)audioSize);
    //直接将数据copy至我们自己分配的内存中
    CMBlockBufferCopyDataBytes(blockBuffer, 0, audioSize, audioData);
        NSData *tempData = [NSData dataWithBytesNoCopy:audioData length:audioSize];
        NSLog(@"data %@",tempData);
    CFRelease(blockBuffer);
}

- (AVStream *)videoEncodeStreaam {
    return _videoEncoder.video_stream;
}

- (void)stopVideoEncode {
    if (_isSucess) {
        self->_videoEncoder.freeEncoder();
    }
    encoder = nil;
}

#pragma mark - private methods
- (void)videoCodec:(AVPacket *)videoPacket {
    if (self.videoDelegate != nil && [self.videoDelegate respondsToSelector:@selector(videoEncodecData:)]) {
        [self.videoDelegate videoEncodecData:videoPacket];
    }
}

#pragma mark - C Functtions
void * VideoEncdeorCallBack(AVPacket *video_packet) {
    [encoder videoCodec:video_packet];
    return NULL;
}

@end
