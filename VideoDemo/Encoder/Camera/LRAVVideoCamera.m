//
//  LRAVVideoCamera.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/9.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LRAVVideoCamera.h"

@interface LRAVVideoCamera ()
{
    dispatch_queue_t                _sampleBufferCallbackQueue;
    int32_t                         _frameRate;
    dispatch_semaphore_t            _frameRenderingSemaphore;

    BOOL                            isFullYUVRange;
    LRImageCameraConfig             *_config;
}
// 处理输入输出设备的数据流动
@property (nonatomic, strong) AVCaptureSession *captureSession;
// 输入输出设备的数据连接
@property (nonatomic, strong) AVCaptureConnection* videoConnection;
// 视频输入设备
@property (nonatomic, strong) AVCaptureDeviceInput *videoCaptureInput;
// 视频输出
@property (nonatomic, strong) AVCaptureVideoDataOutput *videoCaptureOutput;
/** previewLayer */
@property (nonatomic,strong) AVCaptureVideoPreviewLayer *previewLayer;
// 是否应该启用OpenGL渲染，在 willResignActive 时，置为NO, didBecomeActive 时置为YES
@property (nonatomic, assign) BOOL shouldEnableOpenGL;

@end

@implementation LRAVVideoCamera

- (id)initWithCameraConfig:(LRImageCameraConfig *)config {
    if (self = [super init]) {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:NSExtensionHostWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:NSExtensionHostDidBecomeActiveNotification object:nil];
        _frameRate = config.fps;
        _shouldEnableOpenGL = YES;
        _config = config;
        [self initialSession];
        [self initPreviewLayer];
    }
    return self;
}

- (AVCaptureVideoPreviewLayer *)renderLayer {
    return self.previewLayer;
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

- (void)initPreviewLayer {
    if (!_previewLayer) {
        self.previewLayer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:self.captureSession];
    }
}

#pragma mark - 初始化
- (void)initialSession {
    // 初始化session 设备
    self.captureSession = [[AVCaptureSession alloc] init];
    self.videoCaptureInput = [[AVCaptureDeviceInput alloc] initWithDevice:[self frontCamera] error:nil];
    self.videoCaptureOutput = [[AVCaptureVideoDataOutput alloc] init];
    self.videoCaptureOutput.alwaysDiscardsLateVideoFrames = YES;
    // 输出配置
    BOOL supportFullYUVRange = NO;
    NSArray *supportedPixelFormats = _videoCaptureOutput.availableVideoCodecTypes;
    for (NSNumber *currentPielFormat in supportedPixelFormats) {
        if ([currentPielFormat intValue] == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
            supportFullYUVRange = YES;
        }
    }
    
//    kCVPixelFormatType_{长度|序列}{颜色空间}{Planar|BiPlanar}{VideoRange|FullRange}
    if (supportFullYUVRange) {
        [_videoCaptureOutput setVideoSettings:@{(id)kCVPixelBufferPixelFormatTypeKey:@(kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)}];
        isFullYUVRange = YES;
    } else {
        [_videoCaptureOutput setVideoSettings:@{(id)kCVPixelBufferPixelFormatTypeKey:@(kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)}];
        isFullYUVRange = NO;
    }
    [_videoCaptureOutput setSampleBufferDelegate:self queue:[self sampleBufferCallbackQueue]];
    
    // 添加设备
    if ([self.captureSession canAddInput:self.videoCaptureInput]) {
        [self.captureSession addInput:self.videoCaptureInput];
    }
    if ([self.captureSession canAddOutput:self.videoCaptureOutput]) {
        [self.captureSession addOutput:self.videoCaptureOutput];
    }
    
    // 开始配置
    [_captureSession beginConfiguration];
    
    // 视频参数配置
    if ([_captureSession canSetSessionPreset:_config.highPressent]) {
        [_captureSession setSessionPreset:_config.highPressent];
    } else {
        [_captureSession setSessionPreset:_config.normalPresent];
    }
    
    self.videoConnection = [self.videoCaptureOutput connectionWithMediaType:AVMediaTypeVideo];
    [self setRelativeVideoOrientation];
    [self setFrameRate];
    
    // 音频参数配置
    NSError *error;
    [[AVAudioSession sharedInstance] setPreferredIOBufferDuration:0.002 error:&error];
    if (error) {
        NSLog(@"设置audioSessionBuffer error %@",error.localizedDescription);
    }
    
    [[AVAudioSession sharedInstance] setPreferredSampleRate:44100 error:&error];
    if (error) {
        NSLog(@"设置audioSessionSampleRate error %@",error.localizedDescription);
    }
    
    [_captureSession commitConfiguration];
}

#pragma mark - 获取前置
- (AVCaptureDevice *)frontCamera {
    AVCaptureDevice *device = [self cameraWithPosition:AVCaptureDevicePositionFront];
    return device;
}

- (void)setRelativeVideoOrientation {
    self.videoConnection.videoOrientation = AVCaptureVideoOrientationPortrait;
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
        AVCaptureDevicePosition position = [[self.videoCaptureInput device] position];
        
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
            [self.captureSession removeInput:self.videoCaptureInput];
            if ([self.captureSession canAddInput:videoInput]) {
                [self.captureSession addInput:videoInput];
                [self setVideoCaptureInput:videoInput];
            } else {
                [self.captureSession addInput:self.videoCaptureInput];
            }
            
            self.videoConnection = [self.videoCaptureOutput connectionWithMediaType:AVMediaTypeVideo];
            
            AVCaptureVideoStabilizationMode stabilizationModel = AVCaptureVideoStabilizationModeStandard;
            
            BOOL supportStabilization = [self.videoCaptureInput.device.activeFormat isVideoStabilizationModeSupported:stabilizationModel];
            NSLog(@"device active format: %@ , 是否支持防抖 %@",self.videoCaptureInput.device.activeFormat,supportStabilization ? @"support" : @"not support");
            if ([self.videoCaptureInput.device.activeFormat isVideoStabilizationModeSupported:stabilizationModel]) {
                [self.videoConnection setPreferredVideoStabilizationMode:stabilizationModel];
                NSLog(@"+++++++++++ %@",@(self.videoConnection.activeVideoStabilizationMode));
            }
            
            [self setRelativeVideoOrientation];
            [self setFrameRate];
            
            [self.captureSession commitConfiguration];
        } else if (error) {
            result = -1;
        }
    }
    return result;
}

- (void)setFrameRate:(int)frameRate {
    _frameRate = frameRate;
    [self setFrameRate];
}

- (void)setFrameRate {
    if (_frameRate > 0) {
        if ([[self videoCaptureInput].device respondsToSelector:@selector(setActiveVideoMaxFrameDuration:)] &&
            [[self videoCaptureInput].device respondsToSelector:@selector(setActiveVideoMinFrameDuration:)]) {
            NSError *error = nil;
            [[self videoCaptureInput].device lockForConfiguration:&error];
            if (error == nil) {
#if defined(__IPHONE_7_0)
                [[self videoCaptureInput].device setActiveVideoMinFrameDuration:CMTimeMake(1, _frameRate)];
                [[self videoCaptureInput].device setActiveVideoMaxFrameDuration:CMTimeMake(1, _frameRate)];
                
                // 对焦模式
                if ([[self videoCaptureInput].device isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus]) {
                    [[self videoCaptureInput].device setFocusMode:AVCaptureFocusModeContinuousAutoFocus];
                } else if ([[self videoCaptureInput].device isFocusModeSupported:AVCaptureFocusModeAutoFocus]) {
                    [[self videoCaptureInput].device isFocusModeSupported:AVCaptureFocusModeAutoFocus];
                }
#endif
            }
            [[self videoCaptureInput].device unlockForConfiguration];
        } else {
            for (AVCaptureConnection *videoConnection in [self videoCaptureOutput].connections) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                if ([videoConnection respondsToSelector:@selector(setVideoMinFrameDuration:)])
                    videoConnection.videoMinFrameDuration = CMTimeMake(1, _frameRate);
                
                if ([videoConnection respondsToSelector:@selector(setVideoMaxFrameDuration:)])
                    videoConnection.videoMaxFrameDuration = CMTimeMake(1, _frameRate);
#pragma clang diagnostic pop
            }
        }
    } else {
        if ([[self videoCaptureInput].device respondsToSelector:@selector(setActiveVideoMinFrameDuration:)] &&
            [[self videoCaptureInput].device respondsToSelector:@selector(setActiveVideoMaxFrameDuration:)]) {
                    
            NSError *error;
            [[self videoCaptureInput].device lockForConfiguration:&error];
            if (error == nil) {
#if defined(__IPHONE_7_0)
                [[self videoCaptureInput].device setActiveVideoMinFrameDuration:kCMTimeInvalid];
                [[self videoCaptureInput].device setActiveVideoMaxFrameDuration:kCMTimeInvalid];
                        
                // 对焦模式
                if ([[self videoCaptureInput].device isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus]) {
                    [[self videoCaptureInput].device setFocusMode:AVCaptureFocusModeContinuousAutoFocus];
                } else if ([[self videoCaptureInput].device isFocusModeSupported:AVCaptureFocusModeAutoFocus]) {
                    [[self videoCaptureInput].device setFocusMode:AVCaptureFocusModeAutoFocus];
                }
#endif
            }
            [[self videoCaptureInput].device unlockForConfiguration];
        } else {
            for (AVCaptureConnection *videoConnection in [self videoCaptureOutput].connections) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                if ([videoConnection respondsToSelector:@selector(setVideoMinFrameDuration:)])
                    videoConnection.videoMinFrameDuration = kCMTimeInvalid; // This sets videoMinFrameDuration back to default
                        
                if ([videoConnection respondsToSelector:@selector(setVideoMaxFrameDuration:)])
                    videoConnection.videoMaxFrameDuration = kCMTimeInvalid; // This sets videoMaxFrameDuration back to default
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
        if (self.cameraDelegate != nil && [self.cameraDelegate respondsToSelector:@selector(videoCameraDidOutputSampleBuffer:)] && output == self.videoCaptureOutput) {
            [self.cameraDelegate videoCameraDidOutputSampleBuffer:sampleBuffer];
        }
//        if (self.cameraDelegate != nil && [self.cameraDelegate respondsToSelector:@selector(videoCameraDidOutputSampleBuffer:)] && output == self.audioCaptureOutput) {
//            [self.cameraDelegate audioCameraDidOutputSampleBuffer:sampleBuffer];
//        }
    }
}

- (AVCaptureDevicePosition)cameraPosition {
    return [[_videoCaptureInput device] position];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    if ([_captureSession isRunning]) {
        [_captureSession stopRunning];
    }
    [_captureSession removeInput:self.videoCaptureInput];
    [_captureSession removeOutput:self.videoCaptureOutput];
    [_videoCaptureOutput setSampleBufferDelegate:nil queue:dispatch_get_main_queue()];
    NSLog(@"Camera Dealloc");
}

@end
