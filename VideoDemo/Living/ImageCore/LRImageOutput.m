//
//  LRImageOutput.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRImageOutput.h"

void runSyncOnVideoProcessingQueue(void (^block)(void)) {
    dispatch_queue_t videoProcessingQueue = [LRImageContext sharedContextQueue];
#if !OS_OBJECT_USE_OBJC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (dispatch_get_current_queue() == videoProcessingQueue)
#pragma clang diagnostic pop
#else
        if (dispatch_get_specific([LRImageContext contextKey]))
#endif
        {
            block();
        } else {
            dispatch_sync(videoProcessingQueue, block);
        }
}

void runAsyncOnVideoProcessingQueue(void (^block)(void)) {
    dispatch_queue_t videoProcessingQueue = [LRImageContext sharedContextQueue];
    
#if !OS_OBJECT_USE_OBJC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (dispatch_get_current_queue() == videoProcessingQueue)
#pragma clang diagnostic pop
#else
        if (dispatch_get_specific([LRImageContext contextKey]))
#endif
        {
            block();
        } else {
            dispatch_async(videoProcessingQueue, block);
        }
}

void runSyncOnContextQueue(LRImageContext *context, void (^block)(void)) {
    dispatch_queue_t videoProcessingQueue = [context contextQueue];
#if !OS_OBJECT_USE_OBJC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (dispatch_get_current_queue() == videoProcessingQueue)
#pragma clang diagnostic pop
#else
        if (dispatch_get_specific([LRImageContext contextKey]))
#endif
        {
            block();
        } else {
            dispatch_sync(videoProcessingQueue, block);
        }
}

void runAsyncOnContextQueue(LRImageContext *context, void (^block)(void)) {
    dispatch_queue_t videoProcessingQueue = [context contextQueue];
    
#if !OS_OBJECT_USE_OBJC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (dispatch_get_current_queue() == videoProcessingQueue)
#pragma clang diagnostic pop
#else
        if (dispatch_get_specific([LRImageContext contextKey]))
#endif
        {
            block();
        } else {
            dispatch_async(videoProcessingQueue, block);
        }
}

@implementation LRImageOutput

- (instancetype)init {
    if (self = [super init]) {
        targets = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)dealloc {
    [self removeAllTargets];
}

- (LRImageTextureFrame *)framebufferForOutput {
    return outputTexture;
}

- (void)setInputTextureForTarget:(id<LRImageInput>)target {
    [target setInputTexture:[self framebufferForOutput]];
}

- (NSArray *)targets {
    return [NSArray arrayWithArray:targets];
}

- (void)addTarget:(id<LRImageInput>)newTarget {
    [targets addObject:newTarget];
}

- (void)removeTarget:(id<LRImageInput>)targetToRemove {
    if (![targets containsObject:targetToRemove]) {
        return;
    }
    runSyncOnVideoProcessingQueue(^{
        [self->targets removeObject:targetToRemove];
    });
}

- (void)removeAllTargets {
    runSyncOnVideoProcessingQueue(^{
        [self->targets removeAllObjects];
    });
}

@end
