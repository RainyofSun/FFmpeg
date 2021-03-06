//
//  LRVideoAudioMuxer.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/22.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
/*
 tbn：对应容器中的时间基。值是AVStream.time_base的倒数
 tbc：对应编解码器中的时间基。值是AVCodecContext.time_base的倒数
 tbr：从视频流中猜算得到，可能是帧率或场率(帧率的2倍)
 */

#include "LRVideoAudioMuxer.hpp"
#include "unistd.h"

#define kModuleName "LRAVMux"

// 准备混流
bool LRVideoAudioMuxer::prepareForMux(const char *muxFilePath, AVStream *video_stream, AVStream *audio_stream) {

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
    
    ret = this->buildMuxVideoStream(video_stream);
    if (ret < 0) {
        return false;
    }
    
    ret = this->buildMuxAudioStream(audio_stream);
    if (ret < 0) {
        return false;
    }
    
    ret = this->writeFileHeader();
    if (ret < 0 ) {
        return false;
    }
    return true;
}

// 追加视频数据
void LRVideoAudioMuxer::addVideoData(AVPacket *video_pkt) {
    pthread_mutex_lock(&m_muxLock);
//    printf("add video Buff = %p data = %s\n",video_pkt->buf,video_pkt->data);
    if (video_pkt->pts == AV_NOPTS_VALUE) {
        int64_t calc_durtion = (double)AV_TIME_BASE/av_q2d(this->in_v_frame_rate);
        video_pkt->pts = (double)(this->frame_index * calc_durtion)/(double)(av_q2d(this->in_v_stream_time) * AV_TIME_BASE);
        video_pkt->dts = video_pkt->pts;
        video_pkt->duration = (double)calc_durtion/(double)(av_q2d(this->in_v_stream_time) * AV_TIME_BASE);
        this->frame_index ++;
    }
    printf("add video pts = %lld\n",video_pkt->pts);
    
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
    pthread_mutex_unlock(&m_muxLock);
}

// 追加音频数据
void LRVideoAudioMuxer::addAudioData(AVPacket *audio_pkt) {
    pthread_mutex_lock(&m_muxLock);
//    printf("add audio Buff = %p data = %s\n",audio_pkt->buf,audio_pkt->data);
    
    if (audio_pkt->pts == AV_NOPTS_VALUE) {
        int64_t calc_durtion = (double)AV_TIME_BASE/av_q2d(this->in_a_frame_rate);
        audio_pkt->pts = (double)(this->frame_index * calc_durtion)/(double)(av_q2d(this->in_a_stream_time) * AV_TIME_BASE);
        audio_pkt->dts = audio_pkt->pts;
        audio_pkt->duration = (double)calc_durtion/(double)(av_q2d(this->in_a_stream_time) * AV_TIME_BASE);
        this->frame_index ++;
    }
    printf("add audio pts = %lld\n",audio_pkt->pts);
    
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
    pthread_mutex_unlock(&m_muxLock);
}

// 销毁混流器
void LRVideoAudioMuxer::freeMuxHander() {
    printf("mux 开始析构\n");
    if (this->hasFilePath) {
        // 写文件尾
        if (this->writeHeaderSeccess) {
            int ret = av_write_trailer(this->format_context);
            if (ret < 0) {
                printf("文件尾写入失败\n");
            }
        }
    }
    av_free(this->m_video_stream);
    av_free(this->m_audio_stream);
    avio_close(this->format_context->pb);
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
    this->format_context  = NULL;
    this->hasFilePath = strlen(this->muxFilePath) != 0;
    this->writeHeaderSeccess = true;
    this->frame_index  = 0;
    
    this->m_audioListPacket.initPacketList();
    this->m_videoListPacket.initPacketList();
}

int LRVideoAudioMuxer::configureFFmpegFormat() {
    if (this->hasFilePath) {
        avformat_alloc_output_context2(&format_context, NULL, NULL, this->muxFilePath);
        if (!format_context) {
            printf("Could not create output context\n");
            return -1;
        }
        av_dict_set(&format_context->metadata, "description", "mux.mp4", 0);
        ofmt = format_context->oformat;
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
    ret = avio_open(&format_context->pb, this->muxFilePath, AVIO_FLAG_WRITE);
    if (ret < 0) {
        printf("打开输出文件失败");
        return ret;
    }
    return ret;
}

// 初始化视频
int LRVideoAudioMuxer::buildMuxVideoStream(AVStream *video_stream) {
    this->in_v_stream_time = video_stream->time_base;
    this->in_v_frame_rate  = video_stream->r_frame_rate;
    
    int ret = 0;
    
    this->m_video_stream   = avformat_new_stream(format_context, video_stream->codec->codec);
    this->m_video_stream->time_base = video_stream->time_base;
    
    if (!this->m_video_stream) {
        printf("Failed allocating output stream\n");
        return -1;
    }
    
    // 复制流参数
    ret = avcodec_parameters_copy(this->h264Ctx->par_in, video_stream->codecpar);
    ret = avcodec_parameters_from_context(this->m_video_stream->codecpar, video_stream->codec);
    if (ret < 0) {
        printf("Failed to copy context from input to output stream codec context\n");
        return -1;
    }
    
    this->h264Ctx->time_base_in = video_stream->time_base;
    // 初始化过滤器上下文
    ret = av_bsf_init(this->h264Ctx);
    return ret;
}

// 初始化音频
int LRVideoAudioMuxer::buildMuxAudioStream(AVStream *audio_stream) {
    this->in_a_stream_time = audio_stream->time_base;
    this->in_a_frame_rate  = audio_stream->r_frame_rate;
    
    int ret = 0;
    
    this->m_audio_stream = avformat_new_stream(format_context, audio_stream->codec->codec);
    this->m_audio_stream->time_base.num = 1;
    this->m_audio_stream->time_base.den = audio_stream->codec->sample_rate;
    
    if (!this->m_audio_stream) {
        printf("Failed allocating output stream\n");
        return -1;
    }
    
    // 复制流参数
    ret = avcodec_parameters_copy(this->aacCtx->par_in, audio_stream->codecpar);
    ret = avcodec_parameters_from_context(this->m_audio_stream->codecpar, audio_stream->codec);
    
    if (ret < 0) {
        printf("Failed to copy context from input to output stream codec context\n");
        return false;
    }
    this->aacCtx->time_base_in = audio_stream->time_base;
    // 初始化过滤器上下文
    ret = av_bsf_init(this->aacCtx);
    return ret;
}

// 写文件头
int LRVideoAudioMuxer::writeFileHeader() {
    
    int ret = 0;
    
    printf("==========Output Information==========\n");
        av_dump_format(format_context,0, this->muxFilePath,1);
    printf("======================================\n");
    
    // 写文件头
    ret = avformat_write_header(format_context, NULL);
    if (ret < 0) {
        this->writeHeaderSeccess = false;
        printf("文件头写入失败 %s\n",av_err2str(ret));
    }
    return ret;
}

// 获取视频流时间戳
double LRVideoAudioMuxer::GetVideoStreamTimeInSecs() {

    return 0.0;
}

// 获取音频流时间戳
double LRVideoAudioMuxer::GetAudioStreamTimeInSecs() {
    
    return 0.0;
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
        AVPacket *pkt = av_packet_alloc();
        // 比较音视频pts，大于0表示视频帧在前，音频需要连续编码。小于0表示，音频帧在前，应该至少编码一帧视频
        int ret = av_compare_ts(video_packt.timeStamp, this->in_v_stream_time, audio_packt.timeStamp, this->in_a_stream_time);
        uint64_t v_show_time = video_packt.timeStamp * av_q2d(this->in_v_stream_time) * AV_TIME_BASE;
        uint64_t a_show_time = audio_packt.timeStamp * av_q2d(this->in_a_stream_time) * AV_TIME_BASE;
        uint64_t dis;
        if (ret <= 0) {
            dis = a_show_time - v_show_time;
            if (dis > 20000) {  // 丢帧之后有花屏
                printf("执行丢帧 正差值 = %llu \n",dis);
//                av_free(video_packt.pkt_data);
//                av_packet_unref(pkt);
//                video_packt.timeStamp = 0;
//                continue;
            }
            printf("时间戳比较 ret = %d video 显示时间 = %llu audio 显示时间 = %llu 正差值 = %llu \n",ret,v_show_time,a_show_time,dis);
        } else {
            dis = v_show_time - a_show_time;
            printf("时间戳比较 ret = %d video 显示时间 = %llu audio 显示时间 = %llu 负差值 = %llu \n",ret,v_show_time,a_show_time,dis);
        }
        if (ret <= 0) {
            printf("video Buff = %p pts = %lld\n",video_packt.pkt_data->buf,video_packt.pkt_data->pts);
            if (av_bsf_send_packet(this->h264Ctx, video_packt.pkt_data) < 0) {
                av_free(video_packt.pkt_data);
                continue;
            }
            ret = av_bsf_receive_packet(this->h264Ctx, pkt);
            if (ret < 0) {
                av_free(video_packt.pkt_data);
                av_packet_unref(pkt);
                video_packt.timeStamp = 0;
                continue;
            }
            pkt->pts = av_rescale_q_rnd(pkt->pts, this->in_v_stream_time, this->m_video_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->dts = av_rescale_q_rnd(pkt->dts, this->in_v_stream_time, this->m_video_stream->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->duration = av_rescale_q(pkt->duration, this->in_v_stream_time, this->m_video_stream->time_base);
            pkt->pos = -1;
            pkt->stream_index = this->m_video_stream->index;
            this->productAVPacket(pkt);
            av_free(video_packt.pkt_data->data);
            av_free(video_packt.pkt_data);
            video_packt.timeStamp = 0;
        } else {
            printf("audio Buff = %p pts = %lld\n",audio_packt.pkt_data->buf,audio_packt.pkt_data->pts);
            if (av_bsf_send_packet(this->aacCtx, audio_packt.pkt_data) < 0) {
                av_free(audio_packt.pkt_data);
                continue;
            }
            ret = av_bsf_receive_packet(this->aacCtx, pkt);
            if (ret < 0) {
                av_free(audio_packt.pkt_data);
                av_packet_unref(pkt);
                audio_packt.timeStamp = 0;
                continue;
            }
            pkt->pts = av_rescale_q_rnd(pkt->pts, this->in_a_stream_time, this->m_audio_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->dts = av_rescale_q_rnd(pkt->dts, this->in_a_stream_time, this->m_audio_stream->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->duration = av_rescale_q(pkt->duration, this->in_a_stream_time, this->m_audio_stream->time_base);
            pkt->pos = -1;
            pkt->stream_index = this->m_audio_stream->index;
            this->productAVPacket(pkt);
            av_free(audio_packt.pkt_data->data);
            av_free(audio_packt.pkt_data);
            audio_packt.timeStamp = 0;
        }
    }
}

void LRVideoAudioMuxer::productAVPacket(AVPacket *mux_packet) {

    int ret = av_write_frame(this->format_context, mux_packet);
    if (ret < 0) {
        printf("写入文件失败\n");
    }

    av_packet_unref(mux_packet);
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
