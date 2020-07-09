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
void LRVideoAudioMuxer::addVideoData(MediaVideoPacket video_pkt) {
    pthread_mutex_lock(&m_muxLock);
    
    auto duration = this->video_packet_duration - video_pkt.timeMills;
    // 第一帧时不能计算时长，就按照25帧进行计算
    video_pkt.duration = duration < 0 ? 40 : duration;
    video_packet_duration = video_pkt.timeMills;
    
    bool is_success = this->m_videoListPacket.pushData(video_pkt);
    if (!is_success) {
        av_free(video_pkt.pkt_data->data);
        av_free(video_pkt.pkt_data);
        log4cplus_error(kModuleName, "%s: video list is full, push video packet filured!",__func__);
        pthread_mutex_unlock(&m_muxLock);
        return;
    } else {
//        int videoCount = this->m_videoListPacket.count();
//        printf("视频存储个数 %d\n",videoCount);
    }
    pthread_mutex_unlock(&m_muxLock);
}

// 追加音频数据
void LRVideoAudioMuxer::addAudioData(MediaAudioPacket audio_pkt) {
    pthread_mutex_lock(&m_muxLock);
//    printf("add audio Buff = %p data = %s\n",audio_pkt->buf,audio_pkt->data);
    
    bool is_success = this->m_audioListPacket.pushData(audio_pkt);
    if (!is_success) {
        av_free(audio_pkt.pkt_data->data);
        av_free(audio_pkt.pkt_data);
        log4cplus_error(kModuleName, "%s: video list is full, push video packet filured!",__func__);
        pthread_mutex_unlock(&m_muxLock);
        return;
    } else {
//        int audioCount = this->m_audioListPacket.count();
//        printf("音频存储个数 %d\n",audioCount);
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
    this->video_packet_duration = 0;
    this->last_video_presentation_time_ms = 0;
    this->last_audio_packet_presentation_time_mills = 0.0;
    
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
    this->m_video_stream->time_base = (AVRational){1,10000};
    
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
    return this->last_video_presentation_time_ms / 1000.f;
}

// 获取音频流时间戳
double LRVideoAudioMuxer::GetAudioStreamTimeInSecs() {
    return this->last_audio_packet_presentation_time_mills / 1000.f;
}

// 开始混流
void LRVideoAudioMuxer::dispatchAVData() {
    MediaAudioPacket audio_packt;
    MediaVideoPacket video_packt;
    
    memset(&audio_packt, 0, sizeof(MediaVideoPacket));
    memset(&video_packt, 0, sizeof(MediaVideoPacket));
    
    this->m_audioListPacket.reset();
    this->m_videoListPacket.reset();

    int result;
    while (true) {
        int videoCount = this->m_videoListPacket.count();
        int audioCount = this->m_audioListPacket.count();
        
        if(videoCount == 0 || audioCount == 0) {
            usleep(5*1000);
            log4cplus_debug(kModuleName, "%s: Mux dispatch list: v:%d, a:%d",__func__,videoCount, audioCount);
            continue;
        }
        
        double video_time = this->GetVideoStreamTimeInSecs();
        double audio_time = this->GetAudioStreamTimeInSecs();
        
        AVPacket *pkt = av_packet_alloc();
        printf("合并音视频 video_time = %f audio_time = %f\n",video_time,audio_time);
        if (audio_time < video_time) {
            // 写音频
            this->m_audioListPacket.popData(&audio_packt);
            this->last_audio_packet_presentation_time_mills = audio_packt.position;
//            pkt->dts = pkt->pts = (int64_t)(this->last_audio_packet_presentation_time_mills / 1000.f / av_q2d(this->m_audio_stream->time_base));
//            pkt->duration = 1024;
//            pkt->stream_index = this->m_audio_stream->index;
            if (av_bsf_send_packet(this->aacCtx, audio_packt.pkt_data) < 0) {
                av_free(audio_packt.pkt_data);
                av_packet_unref(pkt);
                delete pkt;
                continue;
            }
            result = av_bsf_receive_packet(this->aacCtx, pkt);
            if (result < 0) {
                av_free(audio_packt.pkt_data);
                av_packet_unref(pkt);
                continue;
            }
            pkt->pts = av_rescale_q_rnd(pkt->pts, this->in_a_stream_time, this->m_audio_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->dts = av_rescale_q_rnd(pkt->dts, this->in_a_stream_time, this->m_audio_stream->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->duration = av_rescale_q(pkt->duration, this->in_a_stream_time, this->m_audio_stream->time_base);
            pkt->pos = -1;
            pkt->stream_index = this->m_video_stream->index;
            printf("写入音频 video_pts %lld last_time %f\n",pkt->pts,this->last_audio_packet_presentation_time_mills);
            this->productAVPacket(pkt);
            av_free(audio_packt.pkt_data->data);
            av_free(audio_packt.pkt_data);
        } else {
            // 写视频
            this->m_videoListPacket.popData(&video_packt);
            this->last_video_presentation_time_ms = video_packt.timeMills;
//            pkt->stream_index = this->m_video_stream->index;
//            int64_t cal_pts = static_cast<int64_t>(this->last_video_presentation_time_ms / 1000.f /av_q2d(this->m_video_stream->time_base));
//            printf("写入视频 video_pts %lld last_time %d\n",cal_pts,this->last_video_presentation_time_ms);
//            pkt->dts = pkt->pts = cal_pts;
            if (av_bsf_send_packet(this->h264Ctx, video_packt.pkt_data) < 0) {
                av_free(video_packt.pkt_data);
                continue;
            }
            result = av_bsf_receive_packet(this->h264Ctx, pkt);
            if (result < 0) {
                av_free(video_packt.pkt_data);
                av_packet_unref(pkt);
                continue;
            }
            pkt->pts = av_rescale_q_rnd(pkt->pts, this->in_a_stream_time, this->m_audio_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->dts = av_rescale_q_rnd(pkt->dts, this->in_a_stream_time, this->m_audio_stream->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt->duration = av_rescale_q(pkt->duration, this->in_a_stream_time, this->m_audio_stream->time_base);
            pkt->pos = -1;
            pkt->stream_index = this->m_audio_stream->index;
            printf("写入视频 video_pts %lld last_time %d\n",pkt->pts,this->last_video_presentation_time_ms);
            this->productAVPacket(pkt);
            av_free(video_packt.pkt_data->data);
            av_free(video_packt.pkt_data);
        }
    }
    

    /*
    int videoCount = this->m_videoListPacket.count();
    int audioCount = this->m_audioListPacket.count();
    if(videoCount == 0 || audioCount == 0) {
        usleep(5*1000);
        log4cplus_debug(kModuleName, "%s: Mux dispatch list: v:%d, a:%d",__func__,videoCount, audioCount);
        return;
    }
    
    if(audio_packt.position == 0) {
        this->m_audioListPacket.popData(&audio_packt);
    }
    
    if(video_packt.timeMills == 0) {
        this->m_videoListPacket.popData(&video_packt);
    }
    AVPacket *pkt = av_packet_alloc();
    // 比较音视频pts，大于0表示视频帧在前，音频需要连续编码。小于0表示，音频帧在前，应该至少编码一帧视频
    int ret = av_compare_ts(video_packt.timeMills, this->in_v_stream_time, audio_packt.position, this->in_a_stream_time);
    uint64_t v_show_time = video_packt.timeMills * av_q2d(this->in_v_stream_time) * AV_TIME_BASE;
    uint64_t a_show_time = audio_packt.position * av_q2d(this->in_a_stream_time) * AV_TIME_BASE;
    uint64_t dis;
    if (ret <= 0) {
        printf("video Buff = %p pts = %lld\n",video_packt.pkt_data->buf,video_packt.pkt_data->pts);
        if (av_bsf_send_packet(this->h264Ctx, video_packt.pkt_data) < 0) {
            av_free(video_packt.pkt_data);
            return;
        }
        ret = av_bsf_receive_packet(this->h264Ctx, pkt);
        if (ret < 0) {
            av_free(video_packt.pkt_data);
            av_packet_unref(pkt);
            video_packt.timeMills = 0;
            return;
        }
        pkt->pts = av_rescale_q_rnd(pkt->pts, this->in_v_stream_time, this->m_video_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt->dts = av_rescale_q_rnd(pkt->dts, this->in_v_stream_time, this->m_video_stream->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt->duration = av_rescale_q(pkt->duration, this->in_v_stream_time, this->m_video_stream->time_base);
        pkt->pos = -1;
        pkt->stream_index = this->m_video_stream->index;
        this->productAVPacket(pkt);
        av_free(video_packt.pkt_data->data);
        av_free(video_packt.pkt_data);
        video_packt.timeMills = 0;
    } else {
        printf("audio Buff = %p pts = %lld\n",audio_packt.pkt_data->buf,audio_packt.pkt_data->pts);
        if (av_bsf_send_packet(this->aacCtx, audio_packt.pkt_data) < 0) {
            av_free(audio_packt.pkt_data);
            return;
        }
        ret = av_bsf_receive_packet(this->aacCtx, pkt);
        if (ret < 0) {
            av_free(audio_packt.pkt_data);
            av_packet_unref(pkt);
            audio_packt.position = 0;
            return;
        }
        pkt->pts = av_rescale_q_rnd(pkt->pts, this->in_a_stream_time, this->m_audio_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt->dts = av_rescale_q_rnd(pkt->dts, this->in_a_stream_time, this->m_audio_stream->time_base,(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt->duration = av_rescale_q(pkt->duration, this->in_a_stream_time, this->m_audio_stream->time_base);
        pkt->pos = -1;
        pkt->stream_index = this->m_audio_stream->index;
        this->productAVPacket(pkt);
        av_free(audio_packt.pkt_data->data);
        av_free(audio_packt.pkt_data);
        audio_packt.position = 0;
    }
     */
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
