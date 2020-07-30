//
//  EncoderHeader.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/11.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef EncoderHeader_h
#define EncoderHeader_h
#include <stdlib.h>
#include <time.h>
#include "log4cplus.h"
#include "TimeTools.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
#include "libavutil/avutil.h"
#include "libswresample/swresample.h"
#include "libavutil/samplefmt.h"
#include "libavutil/common.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libyuv.h"
#ifdef __cplusplus
};
#endif

#ifndef UINT64_C
#define UINT64_C(value)__CONCAT(value,ULL)
#endif
#define byte uint8_t
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define LOGI(...)  printf("  ");printf(__VA_ARGS__);
#ifndef PUBLISH_BITE_RATE
#define PUBLISH_BITE_RATE 64000
#define ENCODE_SIZE 1024

#endif

typedef struct VideoFrame_t {
    unsigned char *buffer;  // YUV420P的图像数据
    int size;               // 图像数据的大小
    int timeMills;          // 所代表的时间戳
    int duration;           // 这一帧图像所代表的时间长度
} VideoFrame;

typedef struct {
    uint8_t *y_frame;
    uint8_t *u_frame;
    uint8_t *v_frame;
    
    int width;
    int height;
    
    int stride_y;
    int stride_u;
    int stride_v;
} I420Buffer;

const int delta = 30;   // 码率控制因子

#endif /* EncoderHeader_h */
