//
//  LRAVPacketList.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "LRAVPacketList.hpp"

#define MAX_MEDIALIST_LENGTH    100

void LRAVPacketList::initPacketList() {
    pthread_mutex_init(&m_lock, NULL);
    this->reset();
}

bool LRAVPacketList::pushData(LRMediaList data) {
    bool ret = false;
    
    pthread_mutex_lock(&m_lock);
    
    if (m_count < MAX_MEDIALIST_LENGTH) {
        if (m_count == 0) {
            m_nextTimeStamp = data.timeStamp;
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

void LRAVPacketList::popData(LRMediaList *mediaList) {
    pthread_mutex_lock(&m_lock);
    if (m_count <= 0) {
        pthread_mutex_unlock(&m_lock);
        return;
    }
    
    std::vector<LRMediaList>:: iterator iterator;
    // 将m_totalList内部第一个元素取出赋值 iterator
    iterator = m_totalList.begin();
    
    mediaList->timeStamp = (*iterator).timeStamp;
    mediaList->pkt_data  = (*iterator).pkt_data;
    mediaList->data_type = (*iterator).data_type;
    
    // 删除第一个元素
    m_totalList.erase(iterator);
    m_count --;
    
    if (m_count == 0) {
        m_nextTimeStamp = 0;
    } else {
        iterator = m_totalList.begin();
        m_nextTimeStamp = iterator->timeStamp;
    }
    
    pthread_mutex_unlock(&m_lock);
}

void LRAVPacketList::reset() {
    pthread_mutex_lock(&m_lock);
    
    m_count = 0;
    m_nextTimeStamp = 0;
    m_statis = 0;
    
    pthread_mutex_unlock(&m_lock);
}

int LRAVPacketList::count() {
    int count = 0;
    pthread_mutex_lock(&m_lock);
    count = m_count;
    pthread_mutex_unlock(&m_lock);
    return count;
}

void LRAVPacketList::flush() {
    // 清空所有元素
    m_totalList.clear();
}

void LRAVPacketList::freePthread() {
    pthread_mutex_destroy(&m_lock);
}
