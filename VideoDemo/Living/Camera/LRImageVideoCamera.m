//
//  LRImageVideoCamera.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRImageVideoCamera.h"
#import "LRImageContext.h"
#import "LRImageCameraRender.h"

// BT.601, which is the standard for SDTV.
GLfloat colorConversion601Default[] = {
    1.164,  1.164, 1.164,
    0.0, -0.392, 2.017,
    1.596, -0.813,   0.0,
};

// BT.601 full range (ref: http://www.equasys.de/colorconversion.html)
GLfloat colorConversion601FullRangeDefault[] = {
    1.0,    1.0,    1.0,
    0.0,    -0.343, 1.765,
    1.4,    -0.711, 0.0,
};

// BT.709, which is the standard for HDTV.
GLfloat colorConversion709Default[] = {
    1.164,  1.164, 1.164,
    0.0, -0.213, 2.112,
    1.793, -0.533,   0.0,
};


GLfloat *colorConversion601 = colorConversion601Default;
GLfloat *colorConversion601FullRange = colorConversion601FullRangeDefault;
GLfloat *colorConversion709 = colorConversion709Default;

@interface LRImageVideoCamera ()

{
    dispatch_queue_t                _sampleBufferCallbackQueue;
    int32_t                         _frameRate;
    dispatch_semaphore_t            _frameRenderingSemaphore;
    
    LRImageCameraRender             *_cameraLoadTextRender;
    LRImageRotationMode             _inputTexRotation;
    
    BOOL                            isFullYUVRange;
    const GLfloat *                 _preferredConversion;
    LRImageCameraConfig             *_config;
}

// 处理输入输出设备的数据流动
@property (nonatomic, strong) AVCaptureSession *captureSession;
// 输入输出设备的数据连接
@property (nonatomic, strong) AVCaptureConnection* connection;
// 输入设备
@property (nonatomic, strong) AVCaptureDeviceInput *captureInput;
// 视频输出
@property (nonatomic, strong) AVCaptureVideoDataOutput *captureOutput;
// 是否应该启用OpenGL渲染，在 willResignActive 时，置为NO, didBecomeActive 时置为YES
@property (nonatomic, assign) BOOL shouldEnableOpenGL;

@end

@implementation LRImageVideoCamera

- (id)initWithCameraConfig:(LRImageCameraConfig *)config {
    if (self = [super init]) {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:NSExtensionHostWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:NSExtensionHostDidBecomeActiveNotification object:nil];
        _frameRate = config.fps;
        _shouldEnableOpenGL = YES;
        _config = config;
        [self initialSession];
        [self updateOrientationSendToTargets];
    }
    return self;
}

- (void)applicationWillResignActive:(NSNotification *)notification {
    self.shouldEnableOpenGL = NO;
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    self.shouldEnableOpenGL = YES;
}

#pragma mark - interface methods
- (void)startCapture {
    if (![_captureSession isRunning]) {
        [_captureSession startRunning];
    }
}

- (void)stopCapture {
    if ([_captureSession isRunning]) {
        [_captureSession stopRunning];
    }
}

#pragma mark - 初始化
- (void)initialSession {
    // 初始化session 设备
    self.captureSession = [[AVCaptureSession alloc] init];
    self.captureInput = [[AVCaptureDeviceInput alloc] initWithDevice:[self frontCamera] error:nil];
    self.captureOutput = [[AVCaptureVideoDataOutput alloc] init];
    self.captureOutput.alwaysDiscardsLateVideoFrames = YES;
    // 输出配置
    BOOL supportFullYUVRange = NO;
    NSArray *supportedPixelFormats = _captureOutput.availableVideoCodecTypes;
    for (NSNumber *currentPielFormat in supportedPixelFormats) {
        if ([currentPielFormat intValue] == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
            supportFullYUVRange = YES;
        }
    }
    
    if (supportFullYUVRange) {
        [_captureOutput setVideoSettings:@{(id)kCVPixelBufferPixelFormatTypeKey:@(kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)}];
        isFullYUVRange = YES;
    } else {
        [_captureOutput setVideoSettings:@{(id)kCVPixelBufferPixelFormatTypeKey:@(kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)}];
        isFullYUVRange = NO;
    }
    [_captureOutput setSampleBufferDelegate:self queue:[self sampleBufferCallbackQueue]];
    
    // 添加设备
    if ([self.captureSession canAddInput:self.captureInput]) {
        [self.captureSession addInput:self.captureInput];
    }
    if ([self.captureSession canAddOutput:self.captureOutput]) {
        [self.captureSession addOutput:self.captureOutput];
    }
    
    // 开始配置
    [_captureSession beginConfiguration];
    
    if ([_captureSession canSetSessionPreset:_config.highPressent]) {
        [_captureSession setSessionPreset:_config.highPressent];
    } else {
        [_captureSession setSessionPreset:_config.normalPresent];
    }
    
    self.connection = [self.captureOutput connectionWithMediaType:AVMediaTypeVideo];
    [self setRelativeVideoOrientation];
    [self setFrameRate];
    
    [_captureSession commitConfiguration];
}

#pragma mark - 获取前置
- (AVCaptureDevice *)frontCamera {
    AVCaptureDevice *device = [self cameraWithPosition:AVCaptureDevicePositionFront];
    return device;
}

- (void)setRelativeVideoOrientation {
    self.connection.videoOrientation = AVCaptureVideoOrientationPortrait;
}

#pragma mark - 获取后置
- (AVCaptureDevice *)backCamera {
    AVCaptureDevice *device = [self cameraWithPosition:AVCaptureDevicePositionBack];
    return device;
}

#pragma mark - 获取摄像头
- (AVCaptureDevice *)cameraWithPosition:(AVCaptureDevicePosition)position {
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        if ([device position] == position) {
            NSError *error = nil;
            if ([device isFocusModeSupported:AVCaptureFocusModeAutoFocus] && [device lockForConfiguration:&error]) {
                [device setFocusMode:AVCaptureFocusModeAutoFocus];
                if ([device isFocusPointOfInterestSupported]) {
                    [device setFocusPointOfInterest:CGPointMake(.5f, .5f)];
                }
                [device unlockForConfiguration];
            }
            return device;
        }
    }
    return nil;
}

#pragma mark - 切换分辨率
- (void)switchResolution {
    [_captureSession beginConfiguration];
    if ([_captureSession.sessionPreset isEqualToString:[NSString stringWithString:_config.normalPresent]]) {
        [_captureSession setSessionPreset:_config.highPressent];
    } else {
        [_captureSession setSessionPreset:_config.normalPresent];
    }
    [_captureSession commitConfiguration];
}

#pragma mark - 切换摄像头
- (int)switchFrontCamera {
    NSUInteger cameraCount = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo].count;
    int result = -1;
    if (cameraCount > 1) {
        NSError *error;
        AVCaptureDeviceInput *videoInput;
        AVCaptureDevicePosition position = [[self.captureInput device] position];
        
        if (position == AVCaptureDevicePositionFront) {
            videoInput = [[AVCaptureDeviceInput alloc] initWithDevice:[self backCamera] error:&error];
            result = 1;
        } else if (position == AVCaptureDevicePositionBack) {
            videoInput = [[AVCaptureDeviceInput alloc] initWithDevice:[self frontCamera] error:&error];
            result = 0;
        } else {
            result = -1;
        }
        
        if (videoInput != nil) {
            [self.captureSession beginConfiguration];
            [self.captureSession removeInput:self.captureInput];
            if ([self.captureSession canAddInput:videoInput]) {
                [self.captureSession addInput:videoInput];
                [self setCaptureInput:videoInput];
            } else {
                [self.captureSession addInput:self.captureInput];
            }
            
            self.connection = [self.captureOutput connectionWithMediaType:AVMediaTypeVideo];
            
            AVCaptureVideoStabilizationMode stabilizationModel = AVCaptureVideoStabilizationModeStandard;
            
            BOOL supportStabilization = [self.captureInput.device.activeFormat isVideoStabilizationModeSupported:stabilizationModel];
            NSLog(@"device active format: %@ , 是否支持防抖 %@",self.captureInput.device.activeFormat,supportStabilization ? @"support" : @"not support");
            if ([self.captureInput.device.activeFormat isVideoStabilizationModeSupported:stabilizationModel]) {
                [self.connection setPreferredVideoStabilizationMode:stabilizationModel];
                NSLog(@"+++++++++++ %@",@(self.connection.activeVideoStabilizationMode));
            }
            
            [self setRelativeVideoOrientation];
            [self setFrameRate];
            
            [self.captureSession commitConfiguration];
        } else if (error) {
            result = -1;
        }
        [self updateOrientationSendToTargets];
    }
    return result;
}

- (void)setFrameRate:(int)frameRate {
    _frameRate = frameRate;
    [self setFrameRate];
}

- (void)setFrameRate {
    if (_frameRate > 0) {
        if ([[self captureInput].device respondsToSelector:@selector(setActiveVideoMaxFrameDuration:)] &&
            [[self captureInput].device respondsToSelector:@selector(setActiveVideoMinFrameDuration:)]) {
            NSError *error = nil;
            [[self captureInput].device lockForConfiguration:&error];
            if (error == nil) {
#if defined(__IPHONE_7_0)
                [[self captureInput].device setActiveVideoMinFrameDuration:CMTimeMake(1, _frameRate)];
                [[self captureInput].device setActiveVideoMaxFrameDuration:CMTimeMake(1, _frameRate)];
                
                // 对焦模式
                if ([[self captureInput].device isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus]) {
                    [[self captureInput].device setFocusMode:AVCaptureFocusModeContinuousAutoFocus];
                } else if ([[self captureInput].device isFocusModeSupported:AVCaptureFocusModeAutoFocus]) {
                    [[self captureInput].device isFocusModeSupported:AVCaptureFocusModeAutoFocus];
                }
#endif
            }
            [[self captureInput].device unlockForConfiguration];
        } else {
            for (AVCaptureConnection *connection in [self captureOutput].connections) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                if ([connection respondsToSelector:@selector(setVideoMinFrameDuration:)])
                    connection.videoMinFrameDuration = CMTimeMake(1, _frameRate);
                
                if ([connection respondsToSelector:@selector(setVideoMaxFrameDuration:)])
                    connection.videoMaxFrameDuration = CMTimeMake(1, _frameRate);
#pragma clang diagnostic pop
            }
        }
    } else {
        if ([[self captureInput].device respondsToSelector:@selector(setActiveVideoMinFrameDuration:)] &&
            [[self captureInput].device respondsToSelector:@selector(setActiveVideoMaxFrameDuration:)]) {
                    
            NSError *error;
            [[self captureInput].device lockForConfiguration:&error];
            if (error == nil) {
#if defined(__IPHONE_7_0)
                [[self captureInput].device setActiveVideoMinFrameDuration:kCMTimeInvalid];
                [[self captureInput].device setActiveVideoMaxFrameDuration:kCMTimeInvalid];
                        
                // 对焦模式
                if ([[self captureInput].device isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus]) {
                    [[self captureInput].device setFocusMode:AVCaptureFocusModeContinuousAutoFocus];
                } else if ([[self captureInput].device isFocusModeSupported:AVCaptureFocusModeAutoFocus]) {
                    [[self captureInput].device setFocusMode:AVCaptureFocusModeAutoFocus];
                }
#endif
            }
            [[self captureInput].device unlockForConfiguration];
        } else {
            for (AVCaptureConnection *connection in [self captureOutput].connections) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                if ([connection respondsToSelector:@selector(setVideoMinFrameDuration:)])
                    connection.videoMinFrameDuration = kCMTimeInvalid; // This sets videoMinFrameDuration back to default
                        
                if ([connection respondsToSelector:@selector(setVideoMaxFrameDuration:)])
                    connection.videoMaxFrameDuration = kCMTimeInvalid; // This sets videoMaxFrameDuration back to default
#pragma clang diagnostic pop
            }
        }
    }
}

- (dispatch_queue_t)sampleBufferCallbackQueue {
    if (!_sampleBufferCallbackQueue) {
        _sampleBufferCallbackQueue = dispatch_queue_create("com.lr.sampleBufferCallbackQueue", DISPATCH_QUEUE_SERIAL);
    }
    return _sampleBufferCallbackQueue;
}

- (int32_t)frameRate {
    return _frameRate;
}

#pragma mark - AVCaptureVideoDataOutputSampleBufferDelegate
- (void)captureOutput:(AVCaptureOutput *)output didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
    if (self.shouldEnableOpenGL) {
        if (dispatch_wait(_frameRenderingSemaphore, DISPATCH_TIME_NOW) != 0) {
            return;
        }
        
        CFRetain(sampleBuffer);
        runAsyncOnVideoProcessingQueue(^{
            [self processVideoSampleBuffer:sampleBuffer];
            CFRelease(sampleBuffer);
            dispatch_semaphore_signal(self->_frameRenderingSemaphore);
        });
    }
}

- (void)processVideoSampleBuffer:(CMSampleBufferRef)sampleBuffer {
    CVImageBufferRef cameraFrame = CMSampleBufferGetImageBuffer(sampleBuffer);
    CFTypeRef colorAttachments = CVBufferGetAttachment(cameraFrame, kCVImageBufferYCbCrMatrixKey, NULL);
    if (colorAttachments != NULL) {
        if (CFStringCompare(colorAttachments, kCVImageBufferYCbCrMatrix_ITU_R_601_4, 0) == kCFCompareEqualTo) {
            if (isFullYUVRange) {
                _preferredConversion = colorConversion601FullRange;
            } else {
                _preferredConversion = colorConversion601;
            }
        } else {
            _preferredConversion = colorConversion709;
        }
    } else {
        if (isFullYUVRange) {
            _preferredConversion = colorConversion601FullRange;
        } else {
            _preferredConversion = colorConversion601;
        }
    }
    
    [LRImageContext userImageProcessingContext];
    // 根据宽高分配一个输出纹理并进行绑定sampleBuffer
    [[self cameraFrameTextureWithSampleBuffer:sampleBuffer aspectRatio:_config.aspectRatio] activeFrameBuffer];
    // 渲染之前，需要将CVPixelBuffer中的YUV数据关联到两个纹理ID上
    [_cameraLoadTextRender renderWithSampleBuffer:sampleBuffer aspectRatio:_config.aspectRatio preferredConverstion:_preferredConversion imageRotation:_inputTexRotation];
    CMTime currentTime = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
    CMSampleTimingInfo timingInfo = kCMTimingInfoInvalid;
    CMSampleBufferGetSampleTimingInfo(sampleBuffer, 0, &timingInfo);
    for (id<LRImageInput> currentTarget in targets) {
        [currentTarget setInputTexture:outputTexture];
        [currentTarget newFrameReadyAtTime:currentTime timingInfo:timingInfo];
    }
}

- (LRImageTextureFrame *)cameraFrameTextureWithSampleBuffer:(CMSampleBufferRef)sampleBuffer aspectRatio:(float)aspectRatio {
    if (!outputTexture) {
        CVImageBufferRef cameraFrame = CMSampleBufferGetImageBuffer(sampleBuffer);
        int bufferHeight = (int)CVPixelBufferGetHeight(cameraFrame);
        int targetWidth = bufferHeight / aspectRatio;
        int targetHeight = bufferHeight;
        outputTexture = [[LRImageTextureFrame alloc] initWithSize:CGSizeMake(targetWidth, targetHeight)];
    }
    return outputTexture;
}

- (void)updateOrientationSendToTargets {
    runSyncOnVideoProcessingQueue(^{
        if ([self cameraPosition] == AVCaptureDevicePositionBack) {
            self->_inputTexRotation = kLRImageRotation;
        } else {
            self->_inputTexRotation = kLRImageFlipHorizonal;
        }
    });
}

- (AVCaptureDevicePosition)cameraPosition {
    return [[_captureInput device] position];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    if ([_captureSession isRunning]) {
        [_captureSession stopRunning];
    }
    [_captureSession removeInput:self.captureInput];
    [_captureSession removeOutput:self.captureOutput];
    [_captureOutput setSampleBufferDelegate:nil queue:dispatch_get_main_queue()];
    NSLog(@"Camera Dealloc");
}

@end
