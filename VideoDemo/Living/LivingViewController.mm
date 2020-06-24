//
//  LivingViewController.m
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/1.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import "LivingViewController.h"
#import "LRImageVideoScheduler.h"
#include <stdio.h>
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
    
#ifdef __cplusplus
};
#endif

@interface LivingViewController ()

/** scheduler */
@property (nonatomic,strong) LRImageVideoScheduler *scheduler;

@end

@implementation LivingViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
    self.title = @"开始直播";
    [self ffmpegMethods];
}

- (void)ffmpegMethods {
    av_register_all();
    avdevice_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVInputFormat *iFormat = av_find_input_format("avfoundation");
    if (iFormat != NULL) {
        AVDictionary *options = NULL;
        av_dict_set(&options, "list_devices", "true", 0);
        av_dict_set(&options, "framerate", "30", 0);
        av_dict_set(&options, "video_size", "640x480", 0);
        av_dict_set(&options, "pixel_format", "uyvy422", 0);
        
        printf("=============================\n");
        int ret = avformat_open_input(&pFormatCtx, "0", iFormat, &options);
        if (ret != 0) {
            printf("couldn't open input stream %s\n",av_err2str(ret));
            avformat_free_context(pFormatCtx);
        }
    } else {
        NSLog(@"未找到设备");
    }
}

- (void)nativeMethods {
    LRImageCameraConfig *connfig = [[LRImageCameraConfig alloc] init];
    connfig.fps = 24.f;
    connfig.highPressent = AVCaptureSessionPreset1280x720;
    self.scheduler = [[LRImageVideoScheduler alloc] initWithFrame:self.view.bounds videoConfig:connfig];
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
