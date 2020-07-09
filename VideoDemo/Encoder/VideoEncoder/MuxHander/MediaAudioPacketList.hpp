//
//  MediaAudioPacketList.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/7/8.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef MediaAudioPacketList_hpp
#define MediaAudioPacketList_hpp

#include <stdio.h>
#include "EncoderHeader.h"
#include <vector>
#include <pthread.h>

using namespace std;

class MediaAudioPacketList {
private:
    
    std::vector<MediaAudioPacket>    m_totalList;
    pthread_mutex_t             m_lock;
    u_int64_t                   m_nextTimeStamp;
    int     m_count;
    int     m_statis;
    
public:
    
    void initPacketList();
    
    bool pushData(MediaAudioPacket data);
    void popData(MediaAudioPacket *mediaList);
    
    void reset();
    int count();
    void flush();
    
    void freePthread();
};

#endif /* MediaAudioPacketList_hpp */
