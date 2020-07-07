//
//  TimeTools.h
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/7/7.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#ifndef TimeTools_h
#define TimeTools_h
#include <sys/time.h>

static inline long getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static inline long getCurrentTimeMills() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static inline long getCurrentTimeSeconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

static inline long long currentTimeMills(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#endif /* TimeTools_h */
