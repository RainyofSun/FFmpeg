//
//  EncoderViewController.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/9.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "EncoderViewController.h"
#import "HardwareDeviceScheduler.h"

@interface EncoderViewController ()

/** captureSchedular */
@property (nonatomic,strong) HardwareDeviceScheduler *captureSchedular;

@end

@implementation EncoderViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
    self.title = @"开始采集编码";
    [self nativeMethods];
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
    [self.captureSchedular stopCapture];
}

- (void)nativeMethods {
    LRImageCameraConfig *config = [[LRImageCameraConfig alloc] init];
    // 视频参数
    config.fps = 24.f;
    config.highPressent = AVCaptureSessionPreset1280x720;
    config.videoWidth = 720;
    config.videoHeight = 1280;
    config.videoBitRate = 1536*1000;
    config.frameRate = 30;
    config.videoFilePath = @"video.h264";
    // 音频参数
    config.sampleRate = 44100;
    config.channelsPerFrame = 2;
    config.bitPerChannel = 16;
    config.framesPerPacket = 1;
    config.audioBitRate = 320000;
    config.audioFilePath = @"vocal.aac";
    
    self.captureSchedular = [[HardwareDeviceScheduler alloc] initWithHardwareDeviceConfig:config];
    AVCaptureVideoPreviewLayer *previewLayer = [self.captureSchedular renderLayer];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        previewLayer.frame = self.view.frame;
    });
    [self.view.layer addSublayer:previewLayer];
    [self.captureSchedular startCapture];
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
