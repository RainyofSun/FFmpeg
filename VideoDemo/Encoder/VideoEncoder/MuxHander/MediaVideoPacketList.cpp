//
//  MediaVideoPacketList.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "MediaVideoPacketList.hpp"

#define MAX_MEDIALIST_LENGTH    100 * 1024 * 1024

void MediaVideoPacketList::initPacketList() {
    pthread_mutex_init(&m_lock, NULL);
    this->reset();
}

bool MediaVideoPacketList::pushData(MediaVideoPacket data) {
    bool ret = false;
    
    pthread_mutex_lock(&m_lock);
    
    if (m_count < MAX_MEDIALIST_LENGTH) {
        if (m_count == 0) {
            m_nextTimeStamp = data.timeMills;
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

void MediaVideoPacketList::popData(MediaVideoPacket *mediaList) {
    pthread_mutex_lock(&m_lock);
    if (m_count <= 0) {
        pthread_mutex_unlock(&m_lock);
        return;
    }
    
    std::vector<MediaVideoPacket>:: iterator iterator;
    // 将m_totalList内部第一个元素取出赋值 iterator
    iterator = m_totalList.begin();
    
    mediaList->timeMills = (*iterator).timeMills;
    mediaList->pkt_data  = (*iterator).pkt_data;
    
    // 删除第一个元素
    m_totalList.erase(iterator);
    m_count --;
    
    if (m_count == 0) {
        m_nextTimeStamp = 0;
    } else {
        iterator = m_totalList.begin();
        m_nextTimeStamp = iterator->timeMills;
    }
    
    pthread_mutex_unlock(&m_lock);
}

void MediaVideoPacketList::reset() {
    pthread_mutex_lock(&m_lock);
    
    m_count = 0;
    m_nextTimeStamp = 0;
    m_statis = 0;
    
    pthread_mutex_unlock(&m_lock);
}

int MediaVideoPacketList::count() {
    int count = 0;
    pthread_mutex_lock(&m_lock);
    count = m_count;
    pthread_mutex_unlock(&m_lock);
    return count;
}

void MediaVideoPacketList::flush() {
    // 清空所有元素
    m_totalList.clear();
}

void MediaVideoPacketList::freePthread() {
    pthread_mutex_destroy(&m_lock);
}
