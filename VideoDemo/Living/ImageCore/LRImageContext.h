//
//  LRImageContext.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <OpenGLES/EAGL.h>
#import <CoreMedia/CoreMedia.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import "LRImageTextureFrame.h"

NS_ASSUME_NONNULL_BEGIN

#define TEXTURE_FRAME_ASPECT_RATIO                                  16.0/9.0f

typedef enum {kLRImageRotation,kLRImageFlipHorizonal} LRImageRotationMode;

@protocol LRImageInput <NSObject>

- (void)newFrameReadyAtTime:(CMTime)frameTime timingInfo:(CMSampleTimingInfo)timingInfo;
- (void)setInputTexture:(LRImageTextureFrame *)textureFrame;

@end

@interface LRImageContext : NSObject

@property (nonatomic,readonly) dispatch_queue_t contextQueue;
@property (nonatomic,readonly,retain) EAGLContext *context;
@property (readonly) CVOpenGLESTextureCacheRef coreVideoTextureCache;

+ (void *)contextKey;

+ (LRImageContext *)sharedImageProcessContext;

+ (BOOL)supportFastTextureUpload;

+ (dispatch_queue_t)sharedContextQueue;

+ (void)userImageProcessingContext;

- (CVOpenGLESTextureCacheRef)coreVideoTextureCache;

- (void)useSharedGroup:(EAGLSharegroup *)shareGroup;

- (void)useAsCurrentContext;

@end

NS_ASSUME_NONNULL_END
