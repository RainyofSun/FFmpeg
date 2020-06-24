//
//  LRVideoAudioMuxHander.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/19.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "LRVideoAudioMuxHander.hpp"
#include <unistd.h>

#define kModuleName "LRAVMux"

// 准备混流器
bool LRVideoAudioMuxHander::prepareForMux(const char *muxFilePath) {
    this->muxFilePath = muxFilePath;
    this->initCodec();
    this->initGlobalVar();
    int err = 0;
    err = this->configureFFmpegWithFormat();
    if (err < 0) {
        return false;
    }
    err = pthread_create(&m_muxThread, NULL, MuxAVPacket,(void *)this);
    if (err != 0) {
        log4cplus_error(kModuleName, "%s: create thread failed: %s",__func__, strerror(err));
        return false;
    }
    return true;
}

// 追加视频帧
void LRVideoAudioMuxHander::addVideoData(AVPacket *video_pkt, AVStream *video_stream) {
    pthread_mutex_lock(&m_muxLock);
    this->m_video_stream = avformat_new_stream(this->m_outputContext, video_stream->codec->codec);
    if (!this->m_video_stream) {
        printf("Failed allocating output stream\n");
        pthread_mutex_unlock(&m_muxLock);
        return;
    }
    this->video_index_out = video_stream->index;
    if (avcodec_parameters_from_context(video_stream->codecpar, this->m_video_stream->codec) < 0) {
        printf("Failed to copy context from input to output stream codec context\n");
        pthread_mutex_unlock(&m_muxLock);
        return;
    }
    if (this->m_outputContext->oformat->flags & AVFMT_GLOBALHEADER) {
        this->m_video_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    LRMediaList item = {0};
    item.pkt_data = video_pkt;//内存
    item.timeStamp = video_pkt->pts;
    item.data_type = LRMuxVideoType;
    
    bool is_success = this->m_videoListPacket.pushData(item);
    if (!is_success) {
        av_free(item.pkt_data->data);
        av_free(item.pkt_data);
        log4cplus_error(kModuleName, "%s: video list is full, push video packet filured!",__func__);
        pthread_mutex_unlock(&m_muxLock);
        return;
    }
    pthread_mutex_unlock(&m_muxLock);
}

// 追加音频帧
void LRVideoAudioMuxHander::addAudioData(AVPacket *audio_pkt, AVStream *audio_stream) {
    pthread_mutex_lock(&m_muxLock);
    this->m_audio_stream = avformat_new_stream(this->m_outputContext, audio_stream->codec->codec);
    if (!this->m_audio_stream) {
        printf("Failed allocating output stream\n");
        pthread_mutex_unlock(&m_muxLock);
        return;
    }
    this->audio_index_out = audio_stream->index;
    if (avcodec_parameters_from_context(audio_stream->codecpar, this->m_audio_stream->codec) < 0) {
        printf("Failed to copy context from input to output stream codec context\n");
        pthread_mutex_unlock(&m_muxLock);
        return;
    }
    if (this->m_outputContext->oformat->flags & AVFMT_GLOBALHEADER) {
        this->m_audio_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    LRMediaList item = {0};
    item.pkt_data = audio_pkt;//内存
    item.timeStamp = audio_pkt->pts;
    item.data_type = LRMuxAudioType;
    
    bool is_success = this->m_audioListPacket.pushData(item);
    if (!is_success) {
        av_free(item.pkt_data->data);
        av_free(item.pkt_data);
        log4cplus_error(kModuleName, "%s: video list is full, push video packet filured!",__func__);
        pthread_mutex_unlock(&m_muxLock);
        return;
    }
    pthread_mutex_unlock(&m_muxLock);
}

// 释放混流器
void LRVideoAudioMuxHander::freeMuxHander() {
    printf("mux 开始析构");
    av_free(this->m_video_stream);
    av_free(this->m_audio_stream);
    avio_close(this->m_outputContext->pb);
    avformat_free_context(this->m_outputContext);
    av_bitstream_filter_close(this->h264bsfc);
    av_bitstream_filter_close(this->aacbsfc);
    this->m_videoListPacket.flush();
    this->m_videoListPacket.freePthread();
    this->m_audioListPacket.flush();
    this->m_audioListPacket.freePthread();

    printf("MUX Dealloc\n");
}

#pragma mark - private methods
void LRVideoAudioMuxHander::initCodec() {
    avcodec_register_all();
    av_register_all();
}

// 初始化参数集
void LRVideoAudioMuxHander::initGlobalVar() {
    
    this->m_audioListPacket.initPacketList();
    this->m_videoListPacket.initPacketList();
    
    this->hasFilePath = strlen(this->muxFilePath) != 0;
    this->writeHeaderSeccess = true;
    this->is_ready_for_head = false;
    
    this->m_baseTime.den    = 1000;
    this->m_baseTime.num    = 1;
    this->video_index_out   = 0;
    this->audio_index_out   = 0;
    this->frame_index       = 0;
}

// 初始化封装格式信息
int LRVideoAudioMuxHander::configureFFmpegWithFormat() {
    if (m_outputContext != NULL) {
        av_free(m_outputContext);
        m_outputContext = NULL;
    }
    
    if (this->hasFilePath) {
        avformat_alloc_output_context2(&m_outputContext, NULL, NULL, this->muxFilePath);
        if (!m_outputContext) {
            printf("Could not create output context\n");
            return -1;
        }
        m_outputFormat = this->m_outputContext->oformat;
        m_outputFormat->audio_codec = AV_CODEC_ID_NONE;
        m_outputFormat->video_codec = AV_CODEC_ID_NONE;
        m_outputContext->nb_streams = 0;
        
        int ret = this->openOutputFile();
        if (ret < 0) {
            return ret;
        }
        
        // 写文件头
        ret = avformat_write_header(this->m_outputContext, NULL);
        if (ret < 0) {
            this->writeHeaderSeccess = false;
            printf("文件头写入失败 %s\n",av_err2str(ret));
            return ret;
        }
    }
    
    this->h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
    this->aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
    
    log4cplus_info(kModuleName, "configure ffmpeg finish.");
    return 0;
}

// 打开输出文件
int LRVideoAudioMuxHander::openOutputFile() {
    int ret = 0;
    ret = avio_open(&m_outputContext->pb, this->muxFilePath, AVIO_FLAG_WRITE);
    if (ret < 0) {
        printf("打开输出文件失败");
        return ret;
    }
    return ret;
}

void LRVideoAudioMuxHander::dispatchAVData() {
    LRMediaList audio_packt;
    LRMediaList video_packt;
    
    memset(&audio_packt, 0, sizeof(LRMediaList));
    memset(&video_packt, 0, sizeof(LRMediaList));
    
    this->m_audioListPacket.reset();
    this->m_videoListPacket.reset();
    
    while (true) {
        
        int video_count = this->m_videoListPacket.count();
        int audio_count = this->m_audioListPacket.count();
        
        if (video_count == 0 || audio_count == 0) {
            usleep(5 *1000);
            log4cplus_debug(kModuleName, "%s: Mux dispatch list: v:%d, a:%d",__func__,video_count, audio_count);
            continue;
        }
        
        if (audio_packt.timeStamp == 0) {
            this->m_audioListPacket.popData(&audio_packt);
        }
        if (video_packt.timeStamp == 0) {
            this->m_videoListPacket.popData(&video_packt);
        }
        
        if (audio_packt.timeStamp >= video_packt.timeStamp) {
            log4cplus_debug(kModuleName, "%s: Mux dispatch input video time stamp = %llu",__func__,video_packt.timeStamp);
            if (video_packt.pkt_data != NULL && video_packt.pkt_data->data != NULL) {
                this->addVideoPacket(video_packt.pkt_data, video_packt.timeStamp);
                av_free(video_packt.pkt_data->data);
                av_free(video_packt.pkt_data);
            } else {
                log4cplus_error(kModuleName, "%s: Mux Video AVPacket data abnormal",__func__);
            }
            video_packt.timeStamp = 0;
        } else {
            log4cplus_debug(kModuleName, "%s: Mux dispatch input audio time stamp = %llu",__func__,audio_packt.timeStamp);
            if (audio_packt.pkt_data != NULL && audio_packt.pkt_data->data != NULL) {
                this->addAudioPacket(audio_packt.pkt_data, audio_packt.timeStamp);
                av_free(audio_packt.pkt_data->data);
                av_free(audio_packt.pkt_data);
            } else {
                log4cplus_error(kModuleName, "%s: Mux audio AVPacket data abnormal",__func__);
            }
        }
    }
}

void LRVideoAudioMuxHander::addVideoPacket(AVPacket *video_packet, u_int64_t time_stamp) {
    pthread_mutex_lock(&m_muxLock);
    
    AVPacket output_packet;
    av_init_packet(&output_packet);
    
    output_packet.size = video_packet->size;
    output_packet.data = video_packet->data;
    output_packet.flags= video_packet->flags;
    output_packet.pts  = av_rescale_q(video_packet->pts, this->m_baseTime, this->m_video_stream->time_base);
    output_packet.dts  = video_packet->pts;
    output_packet.duration = av_rescale_q(video_packet->duration, this->m_baseTime, this->m_video_stream->time_base);
    output_packet.stream_index = 0;
    video_packet->stream_index = 0;
    
    log4cplus_debug(kModuleName, "%s: Add video input packet = %llu timestamp = %llu",__func__,video_packet->pts,output_packet.pts);
    
    this->productAVPacket(&output_packet);
    
    pthread_mutex_unlock(&m_muxLock);
}

void LRVideoAudioMuxHander::addAudioPacket(AVPacket *audio_packet, u_int64_t time_stamp) {
    AVPacket output_packet;
    av_init_packet(&output_packet);
    
    output_packet.size         = audio_packet->size;
    output_packet.data         = audio_packet->data;
    output_packet.flags        = audio_packet->flags;
    output_packet.pts          = av_rescale_q(audio_packet->pts, this->m_baseTime, this->m_audio_stream->time_base);
    output_packet.dts          = output_packet.pts;
    output_packet.duration     = av_rescale_q(audio_packet->duration,this->m_baseTime ,this->m_audio_stream->time_base);
    output_packet.stream_index = 1;
    audio_packet->stream_index      = 1;
    
    log4cplus_debug(kModuleName, "%s: Add audio timestamp = %llu",__func__,output_packet.pts);

    this->productAVPacket(&output_packet);
    
    pthread_mutex_unlock(&m_muxLock);
}

void LRVideoAudioMuxHander::productAVPacket(AVPacket *mux_packet) {
    
    uint8_t *output       = NULL;
    int     len           = 0;
    if (avio_open_dyn_buf(&this->m_outputContext->pb) < 0) {
        return;
    }
    
    if (av_write_frame(this->m_outputContext, mux_packet) < 0) {
        avio_close_dyn_buf(this->m_outputContext->pb, (uint8_t **)(&output));
        if (output != NULL) {
            free(output);
        }
        log4cplus_error(kModuleName, "%s: Error writing output data",__func__);
        return;
    }
    
    len = avio_close_dyn_buf(this->m_outputContext->pb, (uint8_t **)(&output));
    if (len == 0 || output == NULL) {
        log4cplus_debug(kModuleName, "%s: mux len:%d or data abnormal",__func__,len);
        if(output != NULL) {
            av_free(output);
        }
        return;
    }
    
    if (output != NULL) {
        av_free(output);
    }
}

#pragma mark - C Function
// 在C++的类中，普通成员函数不能作为pthread_create的线程函数，如果要作为pthread_create中的线程函数，必须是static !
// 静态成员函数只能访问静态变量或静态函数，通过传递this指针进行调用
void* LRVideoAudioMuxHander::MuxAVPacket(void *arg) {
    pthread_setname_np("LR_MUX_THREAD");
    LRVideoAudioMuxHander *instance = (LRVideoAudioMuxHander *)arg;
    if (instance != NULL) {
        instance->dispatchAVData();
    }
    return NULL;
}
