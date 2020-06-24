//
//  LRVideoH264SoftEncoder.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/9.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRVideoH264SoftEncoder.h"
#include "VideoH264SoftEncoder.hpp"

@implementation LRVideoH264SoftEncoder
{
    VideoH264SoftEncoder videoEncoder;
    bool    isSucess;
    dispatch_queue_t encoderQueue;
}

- (instancetype)init {
    if (self = [super init]) {
        isSucess = videoEncoder.initX264Encoder("", 720, 1280, 1536*1000, 30);
        if (!isSucess) {
            NSLog(@"创建解码器失败");
        }
        encoderQueue = dispatch_queue_create(DISPATCH_QUEUE_SERIAL, NULL);
    }
    return self;
}

- (void)H264VideoEncoderWithSampleBuffer:(CMSampleBufferRef)sampleBuffer {
    if (!isSucess) {
        return;
    }
    dispatch_sync(encoderQueue, ^{
        CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);

        CVPixelBufferLockBaseAddress(imageBuffer, 0);

        UInt8 *bufferPtr    = (UInt8 *)CVPixelBufferGetBaseAddressOfPlane(imageBuffer,0);
        UInt8 *bufferPtr1   = (UInt8 *)CVPixelBufferGetBaseAddressOfPlane(imageBuffer,1);
        size_t width = CVPixelBufferGetWidth(imageBuffer);
        size_t height = CVPixelBufferGetHeight(imageBuffer);

        size_t pYBytes      = CVPixelBufferGetBytesPerRowOfPlane(imageBuffer, 0);
        size_t pUVBytes     = CVPixelBufferGetBytesPerRowOfPlane(imageBuffer, 1);

        UInt8 *yuv420_data = (UInt8 *)malloc(width * height *3/ 2);//buffer to store YUV with layout YYYYYYYYUUVV

        /* convert NV12 data to YUV420*/
        UInt8 *pU = yuv420_data + width * height;
        UInt8 *pV = pU + width * height / 4;
        for(int i = 0; i < height; i++) {
            memcpy(yuv420_data + i * width, bufferPtr + i * pYBytes, width);
        }
        for(int j = 0;j < height/2; j++) {
            for(int i = 0; i < width/2; i++)
            {
                *(pU++) = bufferPtr1[i<<1];
                *(pV++) = bufferPtr1[(i<<1) + 1];
            }
            bufferPtr1 += pUVBytes;
        }

        videoEncoder.encode(yuv420_data);
        CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    });
}

@end
