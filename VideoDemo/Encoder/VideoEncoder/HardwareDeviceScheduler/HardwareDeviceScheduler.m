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

@interface HardwareDeviceScheduler ()<VideoCameraCaptureDelegate,AudioCaptureDelegate>

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

@end

@implementation HardwareDeviceScheduler

- (instancetype)initWithHardwareDeviceConfig:(LRImageCameraConfig *)config {
    if (self = [super init]) {
        self.encoderQueue = dispatch_queue_create(DISPATCH_QUEUE_SERIAL, NULL);
        self.videoCapture = [[LRAVVideoCamera alloc] initWithCameraConfig:config];
        self.audioCapture = [[AudioCapture alloc] initWithAudioConfig:config];
        self.videoEncoder = [[LRVideoH264HDEncoder alloc] initWithVideoEncoderConfig:config];
        self.audioEncoder = [[LRAudioAACHDEncoder alloc] initWithAudioEncoderConfig:config];
        self.videoCapture.cameraDelegate = self;
        self.audioCapture.captureDelegate = self;
    }
    return self;
}

- (AVCaptureVideoPreviewLayer *)renderLayer {
    return [self.videoCapture renderLayer];
}

- (void)startCapture {
    [self.videoCapture startCapture];
    [self.audioCapture startCapture];
}

- (void)stopCapture {
    [self.videoCapture stopCapture];
    [self.audioCapture stopCapture];
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

@end
