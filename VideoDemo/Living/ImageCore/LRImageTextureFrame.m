//
//  LRImageTextureFrame.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRImageTextureFrame.h"
#import "LRImageContext.h"
#import "LRImageOutput.h"

@implementation LRImageTextureFrame
{
    GLuint                              _framebuffer;
    GLuint                              _texture;
    GPUTextureFrameOptions              _textureOptions;
    CGSize                              _size;
    
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
    CVPixelBufferRef renderTarget;
    CVOpenGLESTextureRef renderTexture;
    NSUInteger readLockCount;
#else
#endif
}

- (id)initWithSize:(CGSize)frameBufferSize {
    GPUTextureFrameOptions defaultTextureOptions;
    defaultTextureOptions.minFilter = GL_LINEAR;
    defaultTextureOptions.magFilter = GL_LINEAR;
    defaultTextureOptions.wrapS = GL_CLAMP_TO_EDGE;
    defaultTextureOptions.wrapT = GL_CLAMP_TO_EDGE;
    defaultTextureOptions.internalFormat = GL_RGBA;
    defaultTextureOptions.format = GL_RGBA;
    defaultTextureOptions.type = GL_UNSIGNED_BYTE;
    
    if (!(self = [self initWithSize:frameBufferSize textureOptions:defaultTextureOptions])) {
        return nil;
    }
    return self;
}

- (int)width {
    return _size.width;
}

- (int)height {
    return _size.height;
}

- (GLuint)texture {
    return _texture;
}

- (id)initWithSize:(CGSize)frameBufferSize textureOptions:(GPUTextureFrameOptions)fboTextureOptions {
    if (!(self = [super init])) {
        return nil;
    }
    _size = frameBufferSize;
    _textureOptions = fboTextureOptions;
    [self generateFrameBuffer];
    return self;
}

- (void)activeFrameBuffer {
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glViewport(0, 0, (int)_size.width, (int)_size.height);
}

- (void)dealloc {
    [self destroyFrameBuffer];
}

- (void)generateTexture {
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _textureOptions.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _textureOptions.magFilter);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _textureOptions.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _textureOptions.wrapT);
}

- (void)generateFrameBuffer {
    glGenFramebuffers(1, &_framebuffer);
    glBindBuffer(GL_FRAMEBUFFER, _framebuffer);
    
    if ([LRImageContext supportFastTextureUpload]) {
        CVOpenGLESTextureCacheRef coreVideoTextureCache = [[LRImageContext sharedImageProcessContext] coreVideoTextureCache];
        CFDictionaryRef empty;
        CFMutableDictionaryRef attrs;
        
        empty = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, empty);
        
        CVReturn err = CVPixelBufferCreate(kCFAllocatorDefault, (int)_size.width, (int)_size.height, kCVPixelFormatType_32BGRA, attrs, &renderTarget);
        if (err) {
            NSLog(@"FBO size %f %f",_size.width,_size.height);
        }
        
        err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, coreVideoTextureCache, renderTarget, NULL, GL_TEXTURE_2D, _textureOptions.internalFormat, (int)_size.width, (int)_size.height, _textureOptions.format, _textureOptions.type, 0, &renderTexture);
        
        if (err) {
            NSAssert(NO, @"Error at CVOpenGLESTextureCacheCreateTextureFromImage %d", err);
        }
        
        CFRelease(attrs);
        CFRelease(empty);
        
        glBindTexture(CVOpenGLESTextureGetTarget(renderTexture), CVOpenGLESTextureGetName(renderTexture));
        _texture = CVOpenGLESTextureGetName(renderTexture);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _textureOptions.wrapS);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _textureOptions.wrapT);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, CVOpenGLESTextureGetName(renderTexture), 0);
    } else {
        [self generateTexture];
        glBindTexture(GL_TEXTURE_2D, _texture);
        glTexImage2D(GL_TEXTURE_2D, 0, _textureOptions.internalFormat, (int)_size.width, (int)_size.height, 0, _textureOptions.format, _textureOptions.type, 0);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (GLubyte *)byteBuffer {
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
    CVPixelBufferLockBaseAddress(renderTarget, 0);
    GLubyte * bufferBytes = CVPixelBufferGetBaseAddress(renderTarget);
    CVPixelBufferUnlockBaseAddress(renderTarget, 0);
    return bufferBytes;
#else
    return NULL; // TODO: do more with this on the non-texture-cache side
#endif
}

- (void)destroyFrameBuffer {
    runSyncOnVideoProcessingQueue(^{
        if (self->_framebuffer) {
            glDeleteFramebuffers(1, &self->_framebuffer);
            self->_framebuffer = 0;
        }
        if ([LRImageContext supportFastTextureUpload]) {
            if (self->renderTarget) {
                CFRelease(self->renderTarget);
                self->renderTarget = NULL;
            }
            if (self->renderTexture) {
                CFRelease(self->renderTexture);
                self->renderTexture = NULL;
            }
        } else {
            glDeleteTextures(1, &self->_texture);
        }
    });
}

@end
