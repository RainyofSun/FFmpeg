//
//  LRAVVideoCamera.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/9.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "LRImageCameraConfig.h"

NS_ASSUME_NONNULL_BEGIN

@protocol VideoCameraCaptureDelegate <NSObject>

// Video采集输出
- (void)videoCameraDidOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer;

@end

@interface LRAVVideoCamera : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate,AVCaptureAudioDataOutputSampleBufferDelegate>

/** cameraDelegate */
@property (nonatomic,weak) id<VideoCameraCaptureDelegate> cameraDelegate;

- (id)initWithCameraConfig:(LRImageCameraConfig *)config;
- (AVCaptureVideoPreviewLayer *)renderLayer;
- (void)startCapture;
- (void)stopCapture;
- (void)setFrameRate:(int)frameRate;
- (void)setFrameRate;
/**
 * 切换摄像头
 * @return 0: 前置 1: 后置
*/
- (int)switchFrontCamera;
- (void)switchResolution;

@end

NS_ASSUME_NONNULL_END
