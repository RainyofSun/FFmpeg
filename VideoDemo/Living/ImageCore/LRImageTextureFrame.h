//
//  LRImageTextureFrame.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//  用于将纹理对象和帧缓存对象的创建、绑定、销毁等操作进行封装

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

NS_ASSUME_NONNULL_BEGIN

typedef struct GPUTextureFrameOptions {
    GLenum minFilter;
    GLenum magFilter;
    GLenum wrapS;
    GLenum wrapT;
    GLenum internalFormat;
    GLenum format;
    GLenum type;
}GPUTextureFrameOptions;

@interface LRImageTextureFrame : NSObject

- (id)initWithSize:(CGSize)frameBufferSize;

- (id)initWithSize:(CGSize)frameBufferSize textureOptions:(GPUTextureFrameOptions)fboTextureOptions;

- (void)activeFrameBuffer;

- (GLuint)texture;

- (GLubyte *)byteBuffer;

- (int)width;
- (int)height;

@end

NS_ASSUME_NONNULL_END
