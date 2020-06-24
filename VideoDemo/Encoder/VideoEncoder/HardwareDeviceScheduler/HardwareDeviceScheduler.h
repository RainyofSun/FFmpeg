//
//  HardwareDeviceScheduler.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LRImageCameraConfig.h"

NS_ASSUME_NONNULL_BEGIN

@interface HardwareDeviceScheduler : NSObject

- (instancetype)initWithHardwareDeviceConfig:(LRImageCameraConfig *)config;

- (void)startCapture;
- (void)stopCapture;
- (AVCaptureVideoPreviewLayer *)renderLayer;

// 切换摄像头 0: 前置 1: 后置
- (int)switchFrontCamera;
// 切换分辨率
- (void)switchResolution;

@end

NS_ASSUME_NONNULL_END
