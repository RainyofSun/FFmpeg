//
//  LRAudioAACHDEncoder.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LRImageCameraConfig.h"

NS_ASSUME_NONNULL_BEGIN

@interface LRAudioAACHDEncoder : NSObject

- (instancetype)initWithAudioEncoderConfig:(LRImageCameraConfig *)config;
- (void)AACAudioEncoderWithBytes:(uint8_t *)audioData dataLength:(int)length;

@end

NS_ASSUME_NONNULL_END
