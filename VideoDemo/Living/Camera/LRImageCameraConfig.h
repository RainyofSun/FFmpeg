//
//  LRImageCameraConfig.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/3.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface LRImageCameraConfig : NSObject

// VIDEO
/** highPressent */
@property (nonatomic,strong) AVCaptureSessionPreset highPressent;
/** normalPresent */
@property (nonatomic,strong) AVCaptureSessionPreset normalPresent;
/** fps */
@property (nonatomic,assign) int fps;
/** aspectRatio 默认16:9 */
@property (nonatomic,assign) float aspectRatio;
/** videoWidth */
@property (nonatomic,assign) int videoWidth;
/** videoHeight */
@property (nonatomic,assign) int videoHeight;
/** videoBitRate */
@property (nonatomic,assign) int videoBitRate;
/** frameRate */
@property (nonatomic,assign) int frameRate;

// AUDIO
/** sampleRate */
@property (nonatomic,assign) int sampleRate;
/** channelsPerFrame */
@property (nonatomic,assign) int channelsPerFrame;
/** bitPerChannel */
@property (nonatomic,assign) int bitPerChannel;
/** framesPerPacket */
@property (nonatomic,assign) int framesPerPacket;
/** audioBitRate */
@property (nonatomic,assign) int audioBitRate;

// savePath
/** videoFilePath */
@property (nonatomic,copy) NSString *videoFilePath;
/** audioFilePath */
@property (nonatomic,copy) NSString *audioFilePath;

@end

NS_ASSUME_NONNULL_END
