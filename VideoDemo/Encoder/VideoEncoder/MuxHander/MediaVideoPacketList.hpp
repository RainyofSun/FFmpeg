//
//  MediaVideoPacketList.hpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef LRAVPacketList_hpp
#define LRAVPacketList_hpp

#include <stdio.h>
#include "EncoderHeader.h"
#include <vector>
#include <pthread.h>

using namespace std;

class MediaVideoPacketList {
private:
    
    std::vector<MediaVideoPacket>    m_totalList;
    pthread_mutex_t             m_lock;
    u_int64_t                   m_nextTimeStamp;
    int     m_count;
    int     m_statis;
    
public:
    
    void initPacketList();
    
    bool pushData(MediaVideoPacket data);
    void popData(MediaVideoPacket *mediaList);
    
    void reset();
    int count();
    void flush();
    
    void freePthread();
};

#endif /* LRAVPacketList_hpp */
