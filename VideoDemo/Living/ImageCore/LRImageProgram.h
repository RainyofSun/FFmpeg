//
//  LRImageProgram.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//  用于把OpenGL的program的构建、查找属性、使用等操作d封装

#import <Foundation/Foundation.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES2/gl.h>

NS_ASSUME_NONNULL_BEGIN

#define STRINGIZE(x) #x
#define STRINGIZE2(x) STRINGIZE(x)
#define SHADER_STRING(text) @ STRINGIZE2(text)

@interface LRImageProgram : NSObject

- (void)use;

- (BOOL)link;

- (GLuint)uniformIndex:(NSString *)uniformName;

- (GLuint)attributeIndex:(NSString *)attributeName;

- (void)addAttribute:(NSString *)attributeName;

- (id)initWithVertexShaderString:(NSString *)vShaderString fragmentShaderString:(NSString *)fSahderString;

@end

NS_ASSUME_NONNULL_END
