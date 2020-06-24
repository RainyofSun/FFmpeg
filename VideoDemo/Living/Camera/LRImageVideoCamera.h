//
//  LRImageVideoCamera.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "LRImageOutput.h"
#import "LRImageCameraConfig.h"

NS_ASSUME_NONNULL_BEGIN

@interface LRImageVideoCamera : LRImageOutput<AVCaptureVideoDataOutputSampleBufferDelegate>

- (id)initWithCameraConfig:(LRImageCameraConfig *)config;
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
