//
//  LRImageCameraConfig.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRImageCameraConfig.h"
#import "CommonUtil.h"

@implementation LRImageCameraConfig

- (instancetype)init {
    if (self = [super init]) {
        self.normalPresent = AVCaptureSessionPreset640x480;
        self.aspectRatio = 16.0/9.f;
    }
    return self;
}

#pragma mark - setter
- (void)setVideoFilePath:(NSString *)videoFilePath {
    if (videoFilePath.length) {
        videoFilePath = [CommonUtil documentsPath:videoFilePath];
    }
    _videoFilePath = videoFilePath;
}

- (void)setAudioFilePath:(NSString *)audioFilePath {
    if (audioFilePath.length) {
        audioFilePath = [CommonUtil documentsPath:audioFilePath];
    }
    _audioFilePath = audioFilePath;
}

@end
