//
//  LRVideoAudioMuxer.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/22.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//  TODO:
//  1.检查Mux内内存的释放
//  2.整理mux代码
//  3.检查Video_stream Audio_stream参数 --> mux内文件头写入失败

#include "LRVideoAudioMuxer.hpp"
#include "unistd.h"

#define kModuleName "LRAVMux"

// 准备混流
bool LRVideoAudioMuxer::prepareForMux(const char *muxFilePath) {

    this->muxFilePath = muxFilePath;
    
    this->initCodec();
    this->initGlobalVar();
    int ret = this->initBitStreamFilter();
    if (ret < 0) {
        return false;
    }
    ret = this->configureFFmpegFormat();
    if (ret < 0) {
        return false;
    }
    ret = pthread_create(&m_muxThread, NULL, MuxAVPacket,(void *)this);
    if (ret != 0) {
        log4cplus_error(kModuleName, "%s: create thread failed: %s",__func__, strerror(ret));
        return false;
    }
    return true;
}

// 初始化视频/音频BitStreamFilter
bool LRVideoAudioMuxer::initializationMuxBitStreamFilter(AVStream *video_stream, AVStream *audio_stream) {
    AVStream *m_video_stream = avformat_new_stream(ofmt_ctx, video_stream->codec->codec);
    // 添加解码器属性
    int ret = 0;
    
    ret = avcodec_parameters_copy(this->h264Ctx->par_in, video_stream->codecpar);
    if (!m_video_stream) {
        printf("Failed allocating output stream\n");
        return false;
    }
    
    ret = avcodec_parameters_from_context(m_video_stream->codecpar, video_stream->codec);
//    ret = avcodec_copy_context(m_video_stream->codec, video_stream->codec);
    
    if (ret < 0) {
        printf("Failed to copy context from input to output stream codec context\n");
        return false;
    }
    
    // 初始化过滤器上下文
    ret = av_bsf_init(this->h264Ctx);
    
    AVStream *m_audio_stream = avformat_new_stream(ofmt_ctx, audio_stream->codec->codec);
    // 添加解码器属性
    ret = avcodec_parameters_copy(this->aacCtx->par_in, audio_stream->codecpar);
    
    if (!m_audio_stream) {
        printf("Failed allocating output stream\n");
        return false;
    }
    ret = avcodec_parameters_from_context(m_audio_stream->codecpar, audio_stream->codec);
    
    if (ret < 0) {
        printf("Failed to copy context from input to output stream codec context\n");
        return false;
    }
    
    // 初始化过滤器上下文
    ret = av_bsf_init(this->aacCtx);
    
    // 写文件头
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        this->writeHeaderSeccess = false;
        printf("文件头写入失败 %s\n",av_err2str(ret));
        return false;
    }
    return true;
}

// 追加视频数据
void LRVideoAudioMuxer::addVideoData(AVPacket *video_pkt) {
    pthread_mutex_lock(&m_muxLock);
    printf("add video Buff = %p data = %s\n",video_pkt->buf,video_pkt->data);
    
    LRMediaList item = {0};
    memset(&item, 0, sizeof(LRMediaList));
    item.pkt_data = video_pkt;
    item.data_type = LRMuxVideoType;
    item.timeStamp = video_pkt->pts;
    
    bool is_success = this->m_videoListPacket.pushData(item);
    if (!is_success) {
        av_free(item.pkt_data->data);
        av_free(item.pkt_data);
        log4cplus_error(kModuleName, "%s: video list is full, push video packet filured!",__func__);
        pthread_mutex_unlock(&m_muxLock);
        return;
    }
    av_packet_unref(video_pkt);
    pthread_mutex_unlock(&m_muxLock);
}

// 追加音频数据
void LRVideoAudioMuxer::addAudioData(AVPacket *audio_pkt) {
    pthread_mutex_lock(&m_muxLock);
    printf("add audio Buff = %p data = %s\n",audio_pkt->buf,audio_pkt->data);
    LRMediaList item = {0};
    memset(&item, 0, sizeof(LRMediaList));
    item.pkt_data = audio_pkt;
    item.data_type = LRMuxVideoType;
    item.timeStamp = audio_pkt->pts;
    
    bool is_success = this->m_audioListPacket.pushData(item);
    if (!is_success) {
        av_free(item.pkt_data->data);
        av_free(item.pkt_data);
        log4cplus_error(kModuleName, "%s: video list is full, push video packet filured!",__func__);
        pthread_mutex_unlock(&m_muxLock);
        return;
    }
    av_packet_unref(audio_pkt);
    pthread_mutex_unlock(&m_muxLock);
}

// 销毁混流器
void LRVideoAudioMuxer::freeMuxHander() {
    printf("mux 开始析构");
    avio_close(this->ofmt_ctx->pb);
    avformat_free_context(this->ofmt_ctx);
    av_bsf_free(&this->h264Ctx);
    av_bsf_free(&this->aacCtx);
    this->m_videoListPacket.flush();
    this->m_videoListPacket.freePthread();
    this->m_audioListPacket.flush();
    this->m_audioListPacket.freePthread();

    printf("MUX Dealloc\n");
}

#pragma mark - private methods
void LRVideoAudioMuxer::initCodec() {
    avcodec_register_all();
    av_register_all();
}

int LRVideoAudioMuxer::initBitStreamFilter() {
    // 找到相应解码器的过滤器
    this->h264bsfc = av_bsf_get_by_name("h264_mp4toannexb");
    this->aacbsfc = av_bsf_get_by_name("aac_adtstoasc");
    // 过滤器分配内存
    int ret = 0;
    ret = av_bsf_alloc(this->h264bsfc, &this->h264Ctx);
    if (ret < 0) {
        printf("h264 解码器内存分配失败 \n");
        return ret;
    }
    ret = av_bsf_alloc(this->aacbsfc, &this->aacCtx);
    if (ret < 0) {
        printf("aac 解码器内存分配失败 \n");
        return ret;
    }
    return ret;
}

void LRVideoAudioMuxer::initGlobalVar() {
    this->ofmt      = NULL;
    this->ofmt_ctx  = NULL;
    this->hasFilePath = strlen(this->muxFilePath) != 0;
    this->writeHeaderSeccess = true;
    
    this->m_audioListPacket.initPacketList();
    this->m_videoListPacket.initPacketList();
}

int LRVideoAudioMuxer::configureFFmpegFormat() {
    if (this->hasFilePath) {
        avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, this->muxFilePath);
        if (!ofmt_ctx) {
            printf("Could not create output context\n");
            return -1;
        }
        ofmt = ofmt_ctx->oformat;
        int ret = this->openOutputFile();
        if (ret < 0) {
            return ret;
        }
    }
    
    return 0;
}

// 打开输出文件
int LRVideoAudioMuxer::openOutputFile() {
    int ret = 0;
    ret = avio_open(&ofmt_ctx->pb, this->muxFilePath, AVIO_FLAG_WRITE);
    if (ret < 0) {
        printf("打开输出文件失败");
        return ret;
    }
    return ret;
}

// 开始混流
void LRVideoAudioMuxer::dispatchAVData() {
    LRMediaList audio_packt;
    LRMediaList video_packt;
    
    memset(&audio_packt, 0, sizeof(LRMediaList));
    memset(&video_packt, 0, sizeof(LRMediaList));
    
    this->m_audioListPacket.reset();
    this->m_videoListPacket.reset();
    
    while (true) {
        int videoCount = this->m_videoListPacket.count();
        int audioCount = this->m_audioListPacket.count();
        if(videoCount == 0 || audioCount == 0) {
            usleep(5*1000);
            log4cplus_debug(kModuleName, "%s: Mux dispatch list: v:%d, a:%d",__func__,videoCount, audioCount);
            continue;
        }
        
        if(audio_packt.timeStamp == 0) {
            this->m_audioListPacket.popData(&audio_packt);
        }
        
        if(video_packt.timeStamp == 0) {
            this->m_videoListPacket.popData(&video_packt);
        }
        
        if (audio_packt.timeStamp >= video_packt.timeStamp) {
            printf("video Buff = %p data = %s\n",video_packt.pkt_data->buf,video_packt.pkt_data->data);
            if (video_packt.pkt_data != NULL && video_packt.pkt_data->buf != NULL) {
                
                if (av_bsf_send_packet(this->h264Ctx, video_packt.pkt_data) < 0) {
                    av_free(video_packt.pkt_data->data);
                    av_free(video_packt.pkt_data);
                    continue;
                }
                
                AVPacket *pkt = av_packet_alloc();
                
                while (AVERROR_EOF != av_bsf_receive_packet(this->h264Ctx, pkt)) {
                    this->productAVPacket(pkt);
                }
                av_free(video_packt.pkt_data->data);
                av_free(video_packt.pkt_data);
                av_packet_unref(pkt);
            }
            video_packt.timeStamp = 0;
        } else {
            printf("audio Buff = %p data = %s\n",audio_packt.pkt_data->buf,audio_packt.pkt_data->data);
            if (audio_packt.pkt_data != NULL && audio_packt.pkt_data->buf != NULL) {
                if (av_bsf_send_packet(this->aacCtx, audio_packt.pkt_data) < 0) {
                    av_free(audio_packt.pkt_data->data);
                    av_free(audio_packt.pkt_data);
                    continue;
                }
                
                AVPacket *pkt = av_packet_alloc();
                
                while (AVERROR_EOF != av_bsf_receive_packet(this->aacCtx, pkt)) {
                    this->productAVPacket(pkt);
                }
                av_free(video_packt.pkt_data->data);
                av_free(video_packt.pkt_data);
                av_packet_unref(pkt);
            }
            audio_packt.timeStamp = 0;
        }
//        av_bitstream_filter_filter(this->h264bsfc, this->m_video_stream->codec, NULL, &video_packt.pkt_data->data, &video_packt.pkt_data->size, video_packt.pkt_data->data, video_packt.pkt_data->size, 0);
//        av_bitstream_filter_filter(this->aacbsfc, this->m_audio_stream->codec, NULL, &audio_packt.pkt_data->data, &audio_packt.pkt_data->size, audio_packt.pkt_data->data, audio_packt.pkt_data->size, 0);
    }
}

void LRVideoAudioMuxer::productAVPacket(AVPacket *mux_packet) {
    
    uint8_t *output       = NULL;
    int     len           = 0;
    if (avio_open_dyn_buf(&this->ofmt_ctx->pb) < 0) {
        return;
    }
    
    if (int i = av_write_frame(this->ofmt_ctx, mux_packet)) {
        avio_close_dyn_buf(this->ofmt_ctx->pb, (uint8_t **)(&output));
        if (output != NULL) {
            free(output);
        }
        log4cplus_error(kModuleName, "%s: Error writing output data",__func__);
        return;
    }
    
    len = avio_close_dyn_buf(this->ofmt_ctx->pb, (uint8_t **)(&output));
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
void* LRVideoAudioMuxer::MuxAVPacket(void *arg) {
    pthread_setname_np("LR_MUX_THREAD");
    LRVideoAudioMuxer *instance = (LRVideoAudioMuxer *)arg;
    if (instance != NULL) {
        instance->dispatchAVData();
    }
    return NULL;
}
