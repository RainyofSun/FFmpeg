//
//  AudioViewController.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/18.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "AudioViewController.h"
#import "AudioCapture.h"
#import "LRAudioAACHDEncoder.h"

@interface AudioViewController ()<AudioCaptureDelegate>

/** audioCapture */
@property (nonatomic,strong) AudioCapture *audioCapture;
/** audioEncoder */
@property (nonatomic,strong) LRAudioAACHDEncoder *audioEncoder;

@end

@implementation AudioViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
    self.title = @"音频采集编码";
    [self audioCaptureEncode];
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
    [self.audioCapture stopCapture];
}

- (void)audioCaptureEncode {
    LRImageCameraConfig *config = [[LRImageCameraConfig alloc] init];
    config.sampleRate = 44100;
    config.channelsPerFrame = 2;
    config.bitPerChannel = 16;
    config.framesPerPacket = 1;
    self.audioCapture = [[AudioCapture alloc] initWithAudioConfig:config];
    self.audioCapture.captureDelegate = self;
    [self.audioCapture startCapture];
}

#pragma mark - AudioCaptureDelegate
- (void)receiveRecoderData:(uint8_t *)audioData audioDataLength:(int)length {
    [self.audioEncoder AACAudioEncoderWithBytes:audioData dataLength:length];
}

#pragma mark - lazy
- (LRAudioAACHDEncoder *)audioEncoder {
    if (!_audioEncoder) {
        _audioEncoder = [[LRAudioAACHDEncoder alloc] init];
    }
    return _audioEncoder;
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
