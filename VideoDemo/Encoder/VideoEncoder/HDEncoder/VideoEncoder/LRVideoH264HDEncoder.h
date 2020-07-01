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
#import "EncoderHeader.h"

NS_ASSUME_NONNULL_BEGIN

@protocol VideoEncoderDelegate <NSObject>

- (void)videoEncodecData:(AVPacket *)videoPacket;

@end

@interface LRVideoH264HDEncoder : NSObject

/** videoDelegate */
@property (nonatomic,weak) id<VideoEncoderDelegate> videoDelegate;

- (instancetype)initWithVideoEncoderConfig:(LRImageCameraConfig *)config videoDelegate:(id<VideoEncoderDelegate>)videoDelegate;
- (void)H264VideoEncoderWithSampleBuffer:(CMSampleBufferRef)sampleBuffer;
- (void)AACAudioEncoderWithSampleBuffer:(CMSampleBufferRef)sampleBuffer;
- (AVStream *)videoEncodeStreaam;
- (void)stopVideoEncode;

@end

NS_ASSUME_NONNULL_END
