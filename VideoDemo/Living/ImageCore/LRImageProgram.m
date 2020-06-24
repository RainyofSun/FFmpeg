//
//  LRImageProgram.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRImageProgram.h"

@implementation LRImageProgram
{
    NSMutableArray  *attributes;
    NSMutableArray  *uniforms;
    GLuint          program;
    GLuint          vertShader;
    GLuint          fragShader;
}

- (void)use {
    glUseProgram(program);
}

- (id)initWithVertexShaderString:(NSString *)vShaderString fragmentShaderString:(NSString *)fSahderString {
    if (self = [super init]) {
        attributes = [[NSMutableArray alloc] init];
        uniforms = [[NSMutableArray alloc] init];
        program = glCreateProgram();
        
        if (![self compileShader:&vertShader type:GL_VERTEX_SHADER string:vShaderString]) {
            NSLog(@"Failed to compile vertex shader");
        }
        
        if (![self compileShader:&fragShader type:GL_VERTEX_SHADER string:fSahderString]) {
            NSLog(@"Failed to compile fragment shader");
        }
        
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
    }
    return self;
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type string:(NSString *)shaderString {
    GLint status;
    const GLchar *source;
    
    source = (GLchar *)[shaderString UTF8String];
    if (!source) {
        NSLog(@"Failed to load vertex shader");
        return NO;
    }
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    
    if (status != GL_TRUE) {
        GLint logLength;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(*shader, logLength, &logLength, log);
            if (shader == &vertShader) {
                NSLog(@"vertex shader log is %@",[NSString stringWithFormat:@"%s",log]);
            } else {
                NSLog(@"fragment shader log is %@",[NSString stringWithFormat:@"%s",log]);
            }
            free(log);
        }
    }
    return status == GL_TRUE;
}

- (void)addAttribute:(NSString *)attributeName {
    if (![attributes containsObject:attributeName]) {
        [attributes addObject:attributeName];
        glBindAttribLocation(program, (GLuint)[attributes indexOfObject:attributeName], [attributeName UTF8String]);
    }
}

// END: addattribute
// START: indexmethods
- (GLuint)attributeIndex:(NSString *)attributeName {
    return (GLuint)[attributes indexOfObject:attributeName];
}

- (GLuint)uniformIndex:(NSString *)uniformName {
    return glGetUniformLocation(program, [uniformName UTF8String]);
}

- (BOOL)link {
    GLint status;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        return NO;
    }
    
    if (vertShader) {
        glDeleteShader(vertShader);
        vertShader = 0;
    }
    if (fragShader) {
        glDeleteShader(fragShader);
        fragShader = 0;
    }
    
    return YES;
}

@end
