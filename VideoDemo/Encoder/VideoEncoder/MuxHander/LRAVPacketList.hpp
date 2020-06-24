//
//  LRAVPacketList.hpp
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

typedef enum : int {
    LRMuxVideoType,
    LRMuxAudioType,
} LRMuxMediaType;

struct LRMediaList {
    AVPacket        *pkt_data;
    u_int64_t       timeStamp;
    LRMuxMediaType  data_type;
    bool            extraDataHasChanged;
};

class LRAVPacketList {
private:
    
    std::vector<LRMediaList>    m_totalList;
    pthread_mutex_t             m_lock;
    u_int64_t                   m_nextTimeStamp;
    int     m_count;
    int     m_statis;
    
public:
    
    void initPacketList();
    
    bool pushData(LRMediaList data);
    void popData(LRMediaList *mediaList);
    
    void reset();
    int count();
    void flush();
    
    void freePthread();
};

#endif /* LRAVPacketList_hpp */
