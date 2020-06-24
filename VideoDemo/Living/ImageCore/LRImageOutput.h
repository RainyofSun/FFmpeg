//
//  LRImageOutput.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LRImageTextureFrame.h"
#import "LRImageContext.h"

NS_ASSUME_NONNULL_BEGIN

void runSyncOnVideoProcessingQueue(void (^block)(void));
void runAsyncOnVideoProcessingQueue(void (^block)(void));
void runSyncOnContextQueue(LRImageContext *context, void (^block)(void));
void runAsyncOnContextQueue(LRImageContext *context, void (^block)(void));

@interface LRImageOutput : NSObject
{
    LRImageTextureFrame *outputTexture;
    NSMutableArray *targets;
}

- (void)setInputTextureForTarget:(id<LRImageInput>)target;

- (LRImageTextureFrame *)framebufferForOutput;

- (NSArray *)targets;

- (void)addTarget:(id<LRImageInput>)newTarget;
- (void)removeTarget:(id<LRImageInput>)targetToRemove;

- (void)removeAllTargets;

@end

NS_ASSUME_NONNULL_END
