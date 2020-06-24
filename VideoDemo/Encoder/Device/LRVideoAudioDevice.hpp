//
//  LRVideoAudioDevice.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/12.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef LRVideoAudioDevice_hpp
#define LRVideoAudioDevice_hpp
#include "DeviceHeader.h"

#include <stdio.h>

class LRVideoAudioDevice {
private:
    AVFormatContext    *pFormatCtx;
public:
    /**
     * 调取各平台采集设备
     * return 调取成功或者失败
     */
    int callEachPlatformVideoAudioAcquisitionEquipment(void);
};

#endif /* LRVideoAudioDevice_hpp */
