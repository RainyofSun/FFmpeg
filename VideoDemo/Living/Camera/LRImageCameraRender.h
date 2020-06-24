//
//  LRImageCameraRender.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/4.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LRImageContext.h"

NS_ASSUME_NONNULL_BEGIN

@interface LRImageCameraRender : NSObject

- (BOOL)prepareRender:(BOOL)isFullYUVRange;

- (void)renderWithSampleBuffer:(CMSampleBufferRef)sampleBuffer aspectRatio:(float)aspectRatio preferredConverstion:(const GLfloat *)preferredConversion imageRotation:(LRImageRotationMode)inputTexRotation;

@end

NS_ASSUME_NONNULL_END
