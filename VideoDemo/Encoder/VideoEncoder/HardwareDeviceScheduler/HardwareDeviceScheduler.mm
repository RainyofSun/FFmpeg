//
//  HardwareDeviceScheduler.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "HardwareDeviceScheduler.h"
#import "LRAVVideoCamera.h"
#import "AudioCapture.h"
#import "LRVideoH264HDEncoder.h"
#import "LRAudioAACHDEncoder.h"
#include "LRVideoAudioMuxer.hpp"
#include "LRVideoAudioMuxHander.hpp"

@interface HardwareDeviceScheduler ()<VideoCameraCaptureDelegate,AudioCaptureDelegate,VideoEncoderDelegate,AudioEncoderDelegate>
{
    LRVideoAudioMuxer       mediaMuxHander;
//    LRVideoAudioMuxHander   mediaMuxHander;
}
/** videoCapture */
@property (nonatomic,strong) LRAVVideoCamera *videoCapture;
/** audioCapture */
@property (nonatomic,strong) AudioCapture *audioCapture;
/** videoEncoder */
@property (nonatomic,strong) LRVideoH264HDEncoder *videoEncoder;
/** audioEncoder */
@property (nonatomic,strong) LRAudioAACHDEncoder *audioEncoder;
/** encoderQueue */
@property (nonatomic,strong) dispatch_queue_t encoderQueue;
/** isSuccess */
@property (nonatomic,assign) BOOL isSuccess;
/** videoStream */
@property (nonatomic) AVStream *videoStream;
/** audioStream */
@property (nonatomic) AVStream *audioStream;

@end

@implementation HardwareDeviceScheduler

- (instancetype)initWithHardwareDeviceConfig:(LRImageCameraConfig *)config {
    if (self = [super init]) {
        self.encoderQueue = dispatch_queue_create(DISPATCH_QUEUE_SERIAL, NULL);
        self.videoCapture = [[LRAVVideoCamera alloc] initWithCameraConfig:config];
        self.audioCapture = [[AudioCapture alloc] initWithAudioConfig:config];
        self.videoEncoder = [[LRVideoH264HDEncoder alloc] initWithVideoEncoderConfig:config videoDelegate:self];
        self.audioEncoder = [[LRAudioAACHDEncoder alloc] initWithAudioEncoderConfig:config audioDelegate:self];
        self.videoCapture.cameraDelegate    = self;
        self.audioCapture.captureDelegate   = self;
        self.videoStream = [self.videoEncoder videoEncodeStreaam];
        self.audioStream = [self.audioEncoder encodeAudioStream];
        self.isSuccess = YES;
        if (self.videoStream && self.audioStream) {
            self.isSuccess = mediaMuxHander.prepareForMux([config.muxFilePath cStringUsingEncoding:NSUTF8StringEncoding], self.videoStream, self.audioStream);
        }
        if (!self.isSuccess) {
            NSLog(@"混流器创建失败");
        }
    }
    return self;
}

- (void)dealloc {
    NSLog(@"DEALLOC %@",NSStringFromClass(self.class));
}

- (AVCaptureVideoPreviewLayer *)renderLayer {
    return [self.videoCapture renderLayer];
}

- (void)startCapture {
    if (!self.isSuccess) {
        return;
    }
    [self.videoCapture startCapture];
    [self.audioCapture startCapture];
}

- (void)stopCapture {
    if (!self.isSuccess) {
        return;
    }
    [self.videoCapture stopCapture];
    [self.audioCapture stopCapture];
    [self.videoEncoder stopVideoEncode];
    [self.audioEncoder stopAudioEncoder];
    mediaMuxHander.freeMuxHander();
}

- (int)switchFrontCamera {
    return [self.videoCapture switchFrontCamera];
}

- (void)switchResolution {
    [self.videoCapture switchResolution];
}

#pragma mark - VideoCameraCaptureDelegate
- (void)videoCameraDidOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer {
    dispatch_sync(self.encoderQueue, ^{
        [self.videoEncoder H264VideoEncoderWithSampleBuffer:sampleBuffer];
    });
}

#pragma mark - AudioCaptureDelegate
- (void)audioDidOutputRecoderData:(uint8_t *)audioData audioDataLength:(int)length {
    dispatch_sync(self.encoderQueue, ^{
        [self.audioEncoder AACAudioEncoderWithBytes:audioData dataLength:length];
    });
}

#pragma mark - VideoEncoderDelegate
- (void)videoEncodecData:(AVPacket *)videoPacket {
    if (!self.isSuccess) {
        return;
    }
    mediaMuxHander.addVideoData(videoPacket);
}

#pragma mark - AudioEncoderDelegate
 - (void)audioEncodecData:(AVPacket *)audioPacket {
    if (!self.isSuccess) {
        return;
    }
    mediaMuxHander.addAudioData(audioPacket);
}

@end
