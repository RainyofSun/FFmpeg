//
//  NV12ToYUV420P.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/11.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef NV12ToYUV420P_hpp
#define NV12ToYUV420P_hpp

#include <stdio.h>
#include "EncoderHeader.h"

class NV12ToYUV420P {
private:
    
public:
    /**
     * nv12 转换 yuv420p
     */
    AVFrame* nv12_to_yuv420p(AVFrame *nv12_frame);
    
    /**
     * 420p 转换 nv12
     */
    AVFrame* yuv420p_to_nv12(AVFrame *yuv420p);
    
    /**
     * NV12 转换 I420
     */
    I420Buffer convertNV12BufferToI420Buffer(uint8_t *y_frame,uint8_t *uv_frame,int src_stride_y,int src_stride_uv,int pixel_width,int pixel_height);
    
    /**
     * 释放buffer
     */
    void freeI420Buffer(I420Buffer buffer);
};

#endif /* swsacleNV12ToYUV420P_hpp */
