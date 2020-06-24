//
//  LRVideoH264SoftEncoder.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/9.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface LRVideoH264SoftEncoder : NSObject

- (void)H264VideoEncoderWithSampleBuffer:(CMSampleBufferRef)sampleBuffer;

@end

NS_ASSUME_NONNULL_END
