//
//  DeviceHeader.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/12.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef DeviceHeader_h
#define DeviceHeader_h

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
};
#else
//Linux...
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
#endif

//Show Dshow Device
int show_dshow_device(){
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("dshow");
    printf("========Device Info=============\n");
    int ret = avformat_open_input(&pFormatCtx,"video=dummy",iformat,&options);
    printf("================================\n");
    return ret;
}
 
//Show Dshow Device Option
int show_dshow_device_option(){
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options,"list_options","true",0);
    AVInputFormat *iformat = av_find_input_format("dshow");
    printf("========Device Option Info======\n");
    int ret = avformat_open_input(&pFormatCtx,"video=Integrated Camera",iformat,&options);
    printf("================================\n");
    return ret;
}
 
//Show VFW Device
int show_vfw_device(){
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVInputFormat *iformat = av_find_input_format("vfwcap");
    printf("========VFW Device Info======\n");
    int ret = avformat_open_input(&pFormatCtx,"list",iformat,NULL);
    printf("=============================\n");
    return ret;
}

//Show AVFoundation Device
int show_avfoundation_device(){
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("avfoundation");
    printf("==AVFoundation Device Info===\n");
    int ret = avformat_open_input(&pFormatCtx,"",iformat,&options);
    printf("=============================\n");
    return ret;
}

#endif /* DeviceHeader_h */

