//
//  AudioCapture.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/18.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LRImageCameraConfig.h"

NS_ASSUME_NONNULL_BEGIN

@protocol AudioCaptureDelegate <NSObject>

// Audio采集输出
- (void)audioDidOutputRecoderData:(uint8_t *)audioData audioDataLength:(int)length;

@end

@interface AudioCapture : NSObject

/** captureDelegate */
@property (nonatomic,weak) id<AudioCaptureDelegate> captureDelegate;

- (id)initWithAudioConfig:(LRImageCameraConfig *)config;
- (void)startCapture;
- (void)stopCapture;

@end

NS_ASSUME_NONNULL_END
