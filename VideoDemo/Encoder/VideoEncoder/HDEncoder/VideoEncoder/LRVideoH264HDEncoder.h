//
//  LRVideoH264HDEncoder.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/11.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "LRImageCameraConfig.h"

NS_ASSUME_NONNULL_BEGIN

@interface LRVideoH264HDEncoder : NSObject

- (instancetype)initWithVideoEncoderConfig:(LRImageCameraConfig *)config;
- (void)H264VideoEncoderWithSampleBuffer:(CMSampleBufferRef)sampleBuffer;
- (void)AACAudioEncoderWithSampleBuffer:(CMSampleBufferRef)sampleBuffer;

@end

NS_ASSUME_NONNULL_END
