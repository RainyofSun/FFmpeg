//
//  LRRTMPPushFlow.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/7/28.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef LRRTMPPushFlow_hpp
#define LRRTMPPushFlow_hpp

#include <stdio.h>
#include "EncoderHeader.h"

class LRRTMPPushFlow {
private:
    AVFormatContext *ictx;
    AVFormatContext *octx;
    AVOutputFormat  *ofmt;
    
    const char      *rtmp_push_address;
    const char      *rtmp_push_file_path;
    int             video_index;
    long long       start_time;
    long long       frame_index;
    
    // methods
    void initCodec();
    int  initInputFormat();
    int  initOutputFormat();
    int  writeHeader();
    
public:
    /**
     * 准备推流
     */
    bool prepareForPusher(const char *rtmp_address,const char *file_path);
    
    /**
     * 开始推流
     */
    void startPushFlow();
    
    /**
     * 释放内存
     */
    int freePusher();
};

#endif /* LRRTMPPushFlow_hpp */
