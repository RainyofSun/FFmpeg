//
//  LRImageVideoScheduler.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "LRImageVideoCamera.h"

NS_ASSUME_NONNULL_BEGIN

@interface LRImageVideoScheduler : NSObject

/**
 * 默认自动开启对比度
 */
- (instancetype)initWithFrame:(CGRect)frame videoConfig:(LRImageCameraConfig *)config;
- (instancetype)initWithFrame:(CGRect)frame videoConfig:(LRImageCameraConfig *)config disableAutoContrast:(BOOL)disableAutoContrast;
- (UIView *)previewView;

- (void)startPreviewView;
- (void)stopPreviewView;

- (int)switchFrontBackCamera;



@end

NS_ASSUME_NONNULL_END
