//
//  LRImageVideoScheduler.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRImageVideoScheduler.h"

@interface LRImageVideoScheduler ()

/** videoCamera */
@property (nonatomic,strong) LRImageVideoCamera *videoCamera;

@end

@implementation LRImageVideoScheduler

- (instancetype)initWithFrame:(CGRect)frame videoConfig:(LRImageCameraConfig *)config {
    if (self = [self initWithFrame:frame videoConfig:config disableAutoContrast:NO]) {
        
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame videoConfig:(LRImageCameraConfig *)config disableAutoContrast:(BOOL)disableAutoContrast {
    if (self = [super init]) {
        self.videoCamera = [[LRImageVideoCamera alloc] initWithCameraConfig:config];
        [self.videoCamera startCapture];
    }
    return self;
}

- (UIView *)previewView {
    return nil;
}

- (void)startPreviewView {
    
}

- (void)stopPreviewView {
    
}

- (int)switchFrontBackCamera {
    return [self.videoCamera switchFrontCamera];
}

@end
