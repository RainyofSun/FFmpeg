//
//  MediaAudioPacketList.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/7/8.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "MediaAudioPacketList.hpp"
#define MAX_MEDIALIST_LENGTH    100 * 1024 * 1024

void MediaAudioPacketList::initPacketList() {
    pthread_mutex_init(&m_lock, NULL);
    this->reset();
}

bool MediaAudioPacketList::pushData(MediaAudioPacket data) {
    bool ret = false;
    
    pthread_mutex_lock(&m_lock);
    
    if (m_count < MAX_MEDIALIST_LENGTH) {
        if (m_count == 0) {
            m_nextTimeStamp = data.position;
        }
        //在m_totalList的最后一个向量后插入一个元素
        m_totalList.push_back(data);
        m_count ++;
        
        if (m_statis < m_count) {
            m_statis = m_count;
        }
        
        ret = true;
    }
    
    pthread_mutex_unlock(&m_lock);
    return ret;
}

void MediaAudioPacketList::popData(MediaAudioPacket *mediaList) {
    pthread_mutex_lock(&m_lock);
    if (m_count <= 0) {
        pthread_mutex_unlock(&m_lock);
        return;
    }
    
    std::vector<MediaAudioPacket>:: iterator iterator;
    // 将m_totalList内部第一个元素取出赋值 iterator
    iterator = m_totalList.begin();
    
    mediaList->position = (*iterator).position;
    mediaList->pkt_data  = (*iterator).pkt_data;
    
    // 删除第一个元素
    m_totalList.erase(iterator);
    m_count --;
    
    if (m_count == 0) {
        m_nextTimeStamp = 0;
    } else {
        iterator = m_totalList.begin();
        m_nextTimeStamp = iterator->position;
    }
    
    pthread_mutex_unlock(&m_lock);
}

void MediaAudioPacketList::reset() {
    pthread_mutex_lock(&m_lock);
    
    m_count = 0;
    m_nextTimeStamp = 0;
    m_statis = 0;
    
    pthread_mutex_unlock(&m_lock);
}

int MediaAudioPacketList::count() {
    int count = 0;
    pthread_mutex_lock(&m_lock);
    count = m_count;
    pthread_mutex_unlock(&m_lock);
    return count;
}

void MediaAudioPacketList::flush() {
    // 清空所有元素
    m_totalList.clear();
}

void MediaAudioPacketList::freePthread() {
    pthread_mutex_destroy(&m_lock);
}
