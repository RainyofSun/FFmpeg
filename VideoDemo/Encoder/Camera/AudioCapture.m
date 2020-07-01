//
//  AudioCapture.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/18.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "AudioCapture.h"

@interface AudioCapture ()
{
    AudioStreamBasicDescription audioBaseDes;
    AudioQueueRef   audioQueue;
    AudioQueueBufferRef audioQueueBuffers[3];//音频缓存
}

@end

@implementation AudioCapture

- (id)initWithAudioConfig:(LRImageCameraConfig *)config {
    if (self = [super init]) {
        [self initializationProperties:config];
    }
    return self;
}

- (void)dealloc {
    NSLog(@"DEALLOC : %@",NSStringFromClass(self.class));
}

- (void)startCapture {
    OSStatus status = AudioQueueNewInput(&audioBaseDes, audioQueueInputCallBack, (__bridge void *_Nullable)(self), nil, nil, 0, &audioQueue);
    if (status != 0) {
        NSLog(@"new failed %d",(int)status);
        return;
    }
    // 初始化音频缓冲区
    for (int i = 0; i < 3; i ++) {
        int result = AudioQueueAllocateBuffer(audioQueue, 2048, &audioQueueBuffers[i]);
        NSLog(@"AudioQueueAllocateBuffer i = %d,result = %d", i, result);
        AudioQueueEnqueueBuffer(audioQueue, audioQueueBuffers[i], 0, NULL);
    }
    status = AudioQueueStart(audioQueue, NULL);
    if (status != 0) {
        NSLog(@"start fail %d",(int)status);
        return;
    }
}

- (void)stopCapture {
    AudioQueueStop(audioQueue, YES);
}

#pragma mark - private methods
- (void)initializationProperties:(LRImageCameraConfig *)config {
    
    audioBaseDes.mFormatID      = kAudioFormatLinearPCM;
    audioBaseDes.mFormatFlags   = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    // 采样率
    audioBaseDes.mSampleRate    = config.sampleRate;
    // 声道
    audioBaseDes.mChannelsPerFrame = (UInt32)config.channelsPerFrame;
    // 每一个packet一帧数据
    audioBaseDes.mFramesPerPacket = (UInt32)config.framesPerPacket;
    // 每个采样点16bit量化
    audioBaseDes.mBitsPerChannel = (UInt32)config.bitPerChannel;
    audioBaseDes.mBytesPerFrame = (audioBaseDes.mBitsPerChannel / 8) * audioBaseDes.mChannelsPerFrame;
    audioBaseDes.mBytesPerPacket = audioBaseDes.mBytesPerFrame * audioBaseDes.mFramesPerPacket;
}

// 采集数据回调
void audioQueueInputCallBack(void * __nullable               inUserData,
                             AudioQueueRef                   inAQ,
                             AudioQueueBufferRef             inBuffer,
                             const AudioTimeStamp *          inStartTime,
                             UInt32                          inNumberPacketDescriptions,
                             const AudioStreamPacketDescription * __nullable inPacketDescs) {
    AudioCapture *captuer = (__bridge AudioCapture *)inUserData;
    if (captuer.captureDelegate != nil && [captuer.captureDelegate respondsToSelector:@selector(audioDidOutputRecoderData:audioDataLength:)]) {
        [captuer.captureDelegate audioDidOutputRecoderData:(uint8_t *)inBuffer->mAudioData audioDataLength:(int)inBuffer->mAudioDataByteSize];
    }
//    NSData *data = [NSData dataWithBytes:inBuffer->mAudioData length:inBuffer->mAudioDataByteSize];
//    NSLog(@"采集数据 = %@\n 数据长度 = %d",data,inBuffer->mAudioDataByteSize);
    AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
}

@end
