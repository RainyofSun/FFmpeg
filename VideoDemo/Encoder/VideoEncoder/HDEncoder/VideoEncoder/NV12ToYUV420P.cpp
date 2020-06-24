//
//  NV12ToYUV420P.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/11.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "NV12ToYUV420P.hpp"

AVFrame* NV12ToYUV420P::nv12_to_yuv420p(AVFrame *nv12_frame) {
    int x,y;
    // yuv420p frame alloc
    AVFrame *yuv420p_frame = av_frame_alloc();
    if (!yuv420p_frame) {
        printf("could not alloc yuv420p frame");
        return NULL;
    }
    yuv420p_frame->format = AV_PIX_FMT_YUV420P;
    yuv420p_frame->width = nv12_frame->width;
    yuv420p_frame->height = nv12_frame->height;
    
    // 32为内存对齐需要
    // alloc frame->data memory
    int ret = av_frame_get_buffer(yuv420p_frame, 32);
    if (ret < 0) {
        printf("could not allocate video frame data");
        return NULL;
    }
    
    ret = av_frame_make_writable(yuv420p_frame);
    if (ret < 0) {
        printf("frame could not write");
        return NULL;
    }
    // copy data
    // Y
    if (nv12_frame->linesize[0] == nv12_frame->width) {
        memcpy(yuv420p_frame->data[0], nv12_frame->data[0], nv12_frame->height * nv12_frame->linesize[0]);
    } else {
        for (y = 0; y < yuv420p_frame->height; y ++) {
            for (x = 0; x < yuv420p_frame->width; x ++) {
                yuv420p_frame->data[0][y * yuv420p_frame->linesize[0] + x] = nv12_frame->data[0][y * nv12_frame->linesize[0] + x];
            }
        }
    }
    
    // Cb & Cr
    for (y = 0; y <yuv420p_frame->height/2; y ++) {
        for (x = 0; x < yuv420p_frame->width/2; x ++) {
            yuv420p_frame->data[1][y * yuv420p_frame->linesize[1] + x] = nv12_frame->data[1][y * nv12_frame->linesize[1] + 2 * x];
            yuv420p_frame->data[2][y * yuv420p_frame->linesize[2] + x] = nv12_frame->data[1][y * nv12_frame->linesize[1] + 2 * x + 1];
        }
    }
    return yuv420p_frame;
}

AVFrame* NV12ToYUV420P::yuv420p_to_nv12(AVFrame *yuv420p_frame) {
    int x,y;
    // yuv420p frame alloc
    AVFrame *nv12_frame = av_frame_alloc();
    if (!nv12_frame) {
        printf("could not alloc yuv420p frame");
        return NULL;
    }
    nv12_frame->format = AV_PIX_FMT_NV12;
    nv12_frame->width = yuv420p_frame->width;
    nv12_frame->height = yuv420p_frame->height;
    
    // 32为内存对齐需要
    // alloc frame->data memory
    int ret = av_frame_get_buffer(nv12_frame, 32);
    if (ret < 0) {
        printf("could not allocate video frame data");
        return NULL;
    }
    
    ret = av_frame_make_writable(nv12_frame);
    if (ret < 0) {
        printf("frame could not write");
        return NULL;
    }
    // copy data
    // Y
    if (yuv420p_frame->linesize[0] == yuv420p_frame->width) {
        memcpy(nv12_frame->data[0], yuv420p_frame->data[0], yuv420p_frame->height * yuv420p_frame->linesize[0]);
    } else {
        for (y = 0; y < nv12_frame->height; y ++) {
            for (x = 0; x < nv12_frame->width; x ++) {
                nv12_frame->data[0][y * nv12_frame->linesize[0] + x] = yuv420p_frame->data[0][y * yuv420p_frame->linesize[0] + x];
            }
        }
    }
    
    // Cb & Cr
    for (y = 0; y <nv12_frame->height/2; y ++) {
        for (x = 0; x < nv12_frame->width/2; x ++) {
            nv12_frame->data[1][y * nv12_frame->linesize[1] + 2 * x]        = yuv420p_frame->data[1][y * yuv420p_frame->linesize[1] + x];
            nv12_frame->data[1][y * nv12_frame->linesize[1] + 2 * x + 1]    = yuv420p_frame->data[2][y * yuv420p_frame->linesize[2] + x];
        }
    }
    return yuv420p_frame;
}

I420Buffer NV12ToYUV420P::convertNV12BufferToI420Buffer(uint8_t *y_frame, uint8_t *uv_frame, int src_stride_y, int src_stride_uv, int pixel_width, int pixel_height) {
    
    uint8_t *i420_y = (uint8_t *)malloc(pixel_width * pixel_height * 1.5);
    uint8_t *i420_u = i420_y + pixel_width * pixel_height;
    uint8_t *i420_v = i420_u + pixel_width * pixel_height/4;
    
    int dst_sride_y = (int)pixel_width;
    int dst_sride_u = (int)pixel_width/2;
    int dst_sride_v = (int)pixel_width/2;
    
    int ret = libyuv::NV12ToI420(y_frame, src_stride_y, uv_frame, src_stride_uv, i420_y, dst_sride_y, i420_u, dst_sride_u, i420_v, dst_sride_v, pixel_width, pixel_height);
    
    I420Buffer buffer = {0};
    if (ret != 0) {
        printf("转换错误");
        return buffer;
    }
    
    buffer.y_frame = i420_y;
    buffer.u_frame = i420_u;
    buffer.v_frame = i420_v;
    
    buffer.stride_y = dst_sride_y;
    buffer.stride_u = dst_sride_u;
    buffer.stride_v = dst_sride_v;
    
    buffer.width = pixel_width;
    buffer.height = pixel_height;
    
    return buffer;
}

void NV12ToYUV420P::freeI420Buffer(I420Buffer buffer) {
    free(buffer.y_frame);
    buffer.y_frame = NULL;
    buffer.u_frame = NULL;
    buffer.v_frame = NULL;
}
