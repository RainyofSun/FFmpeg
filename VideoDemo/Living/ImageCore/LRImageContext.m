//
//  LRImageContext.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRImageContext.h"

@interface LRImageContext ()
{
    EAGLSharegroup *_sharegroup;
}
@end

@implementation LRImageContext

@synthesize context = _context;
@synthesize contextQueue = _contextQueue;
@synthesize coreVideoTextureCache = _coreVideoTextureCache;

static void *openGLESContextQueueKey;

- (instancetype)init {
    if (self = [super init]) {
        openGLESContextQueueKey = &openGLESContextQueueKey;
        _contextQueue = dispatch_queue_create("com.esaylive.LRImage.openGLESContextQueue", NULL);
#if OS_OBJECT_USE_OBJC
    dispatch_queue_set_specific(_contextQueue, openGLESContextQueueKey, (__bridge void *)self, NULL);
#endif
    }
    return self;
}

+ (void *)contextKey {
    return openGLESContextQueueKey;
}

// 单例的处理图像的Context
+ (LRImageContext *)sharedImageProcessContext {
    static dispatch_once_t pred;
    static LRImageContext *sharedImageProcessingContext = nil;
    
    dispatch_once(&pred, ^{
        sharedImageProcessingContext = [[[self class] alloc] init];
    });
    return sharedImageProcessingContext;
}

+ (dispatch_queue_t)sharedContextQueue {
    return [[self sharedImageProcessContext] contextQueue];
}

+ (void)userImageProcessingContext {
    [[LRImageContext sharedImageProcessContext] useAsCurrentContext];
}

- (void)useAsCurrentContext {
    EAGLContext *imageProcessingContext = [self context];
    if ([EAGLContext currentContext] != imageProcessingContext) {
        [EAGLContext setCurrentContext:imageProcessingContext];
    }
}

- (void)useSharedGroup:(EAGLSharegroup *)shareGroup {
    _sharegroup = shareGroup;
}

- (EAGLContext *)createContext {
    EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:_sharegroup];
    return context;
}

+ (BOOL)supportFastTextureUpload {
#if TARGET_IPHONE_SIMULATOR
    return NO;
#else
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-pointer-compare"
    return (CVOpenGLESTextureCacheCreate != NULL);
#pragma clang diagnostic pop
    
#endif
}

- (EAGLContext *)context {
    if (_context == nil) {
        _context = [self createContext];
        [EAGLContext setCurrentContext:_context];
        glDisable(GL_DEPTH_TEST);
    }
    return _context;
}

- (CVOpenGLESTextureCacheRef)coreVideoTextureCache {
    if (_coreVideoTextureCache == NULL) {
#if defined(__IPHONE_6_0)
        CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, [self context], NULL, &_coreVideoTextureCache);
#else
        CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, (__bridge void *)[self context], NULL, &_coreVideoTextureCache);
#endif
        if (err) {
            NSAssert(NO, @"Error at CVOpenGLESTextureCacheCreate %d", err);
        }
    }
    return _coreVideoTextureCache;
}

- (void)dealloc {
    if (_coreVideoTextureCache) {
        CFRelease(_coreVideoTextureCache);
        NSLog(@"release _coreVideoCache ...");
    }
}

@end
