//
//  LRAudioAACHDEncoder.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LRImageCameraConfig.h"
#import "EncoderHeader.h"

NS_ASSUME_NONNULL_BEGIN

@protocol AudioEncoderDelegate <NSObject>

- (void)audioEncodecData:(AVPacket *)audioPacket;
- (void)encodeAudioStream:(AVStream *)audioStream;

@end

@interface LRAudioAACHDEncoder : NSObject

/** audioDelegate */
@property (nonatomic,weak) id<AudioEncoderDelegate> audioDelegate;

- (instancetype)initWithAudioEncoderConfig:(LRImageCameraConfig *)config audioDelegate:(id<AudioEncoderDelegate>)audioDelegate;
- (void)AACAudioEncoderWithBytes:(uint8_t *)audioData dataLength:(int)length;

@end

NS_ASSUME_NONNULL_END
