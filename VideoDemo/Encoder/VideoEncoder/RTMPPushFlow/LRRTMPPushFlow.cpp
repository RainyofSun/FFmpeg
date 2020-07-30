//
//  LRRTMPPushFlow.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/7/28.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "LRRTMPPushFlow.hpp"

// 准备推流
bool LRRTMPPushFlow::prepareForPusher(const char *rtmp_address, const char *file_path) {

    this->rtmp_push_address = rtmp_push_address;
    this->rtmp_push_file_path = file_path;
    
    this->initCodec();
    int ret = this->initInputFormat();
    if (ret < 0) {
        return false;
    }
    
    ret = this->initOutputFormat();
    if (ret < 0) {
        return false;
    }
    
    ret = this->writeHeader();
    if (ret < 0) {
        return false;
    }
    
    this->start_time = av_gettime();
    this->frame_index = 0;
    
    return true;
}

// 开始推流
void LRRTMPPushFlow::startPushFlow() {
    int ret;
    AVPacket *pkt = nullptr;
    while (1) {
        AVStream *in_stream, *out_stream;
        ret = av_read_frame(this->ictx, pkt);
        if (ret < 0) {
            break;
        }
        
        /*
        PTS（Presentation Time Stamp）显示播放时间
        DTS（Decoding Time Stamp）解码时间
        */
        //没有显示时间（比如未解码的 H.264 ）
        if (pkt->pts == AV_NOPTS_VALUE) {
            //AVRational time_base：时基。通过该值可以把PTS，DTS转化为真正的时间。
            AVRational time_base1 = ictx->streams[this->video_index]->time_base;

            //计算两帧之间的时间
            /*
            r_frame_rate 基流帧速率  （不是太懂）
            av_q2d 转化为double类型
            */
            int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(ictx->streams[this->video_index]->r_frame_rate);

            //配置参数
            pkt->pts = (double)(this->frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
            pkt->dts = pkt->pts;
            pkt->duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
        }
        
        //延时
        if (pkt->stream_index == this->video_index) {
            AVRational time_base = ictx->streams[this->video_index]->time_base;
            AVRational time_base_q = { 1,AV_TIME_BASE };
            //计算视频播放时间
            int64_t pts_time = av_rescale_q(pkt->dts, time_base, time_base_q);
            //计算实际视频的播放时间
            int64_t now_time = av_gettime() - start_time;

            AVRational avr = ictx->streams[this->video_index]->time_base;
            if (pts_time > now_time) {
                //睡眠一段时间（目的是让当前视频记录的播放时间与实际时间同步）
                av_usleep((unsigned int)(pts_time - now_time));
            }
        }
        
        in_stream = ictx->streams[pkt->stream_index];
        out_stream = octx->streams[pkt->stream_index];

        //计算延时后，重新指定时间戳
        pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base,(AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt->duration = (int)av_rescale_q(pkt->duration, in_stream->time_base, out_stream->time_base);
        //字节流的位置，-1 表示不知道字节流位置
        pkt->pos = -1;

        if (pkt->stream_index == this->video_index) {
            printf("Send %8lld video frames to output URL\n", this->frame_index);
            frame_index++;
        }

        //向输出上下文发送（向地址推送）
        ret = av_interleaved_write_frame(this->octx, pkt);

        if (ret < 0) {
            printf("发送数据包出错\n");
            break;
        }

        //释放
        av_packet_unref(pkt);
    }
}

// 释放内存
int LRRTMPPushFlow::freePusher() {
    
    return 0;
}

#pragma mark - private methods
void LRRTMPPushFlow::initCodec() {
    av_register_all();
    avformat_network_init();
}

// 输入流处理
int LRRTMPPushFlow::initInputFormat() {
    int ret = avformat_open_input(&this->ictx, this->rtmp_push_file_path, 0, NULL);
    if (ret < 0) {
        printf("打开文件头失败\n");
        return ret;
    }
    
    ret = avformat_find_stream_info(this->ictx, 0);
    if (ret < 0) {
        printf("流信息失败\n");
        return ret;
    }
    
    // 打印视频信息
    av_dump_format(this->ictx, 0, this->rtmp_push_file_path, 0);
    
    return ret;
}

// 输出流处理
int LRRTMPPushFlow::initOutputFormat() {
    int ret = avformat_alloc_output_context2(&this->octx, NULL, "flv", this->rtmp_push_address);
    if (ret < 0) {
        printf("输出处理失败\n");
        return ret;
    }
    
    this->ofmt = this->octx->oformat;
    
    for (int i = 0; i < this->ictx->nb_streams; i ++) {
        AVStream *in_stream = this->ictx->streams[i];
        AVStream *out_stream = avformat_new_stream(this->octx, in_stream->codec->codec);
        if (!out_stream) {
            printf("未成功添加音视频流\n");
            ret = -1;
            break;
        }
        // 将编码器上下文信息拷贝到编解码器上下文
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            printf("编码器上下文拷贝失败\n");
            break;
        }
        
        out_stream->codecpar->codec_tag = 0;
        out_stream->codec->codec_tag = 0;
        
        if (this->octx->oformat->flags & AVFMT_GLOBALHEADER) {
            out_stream->codec->flags = out_stream->codec->flags | AV_CODEC_FLAG_GLOBAL_HEADER;
        }
    }
    
    for (int i = 0; i < this->ictx->nb_streams; i ++) {
        if (this->ictx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            this->video_index = i;
            break;
        }
    }
    
    av_dump_format(this->octx, 0, this->rtmp_push_address, 1);
    
    return ret;
}

// 写头信息
int LRRTMPPushFlow::writeHeader() {
    int ret = avio_open(&this->octx->pb, this->rtmp_push_address, 0);
    if (ret < 0) {
        printf("IO打开失败 \n");
        return ret;
    }
    
    ret = avformat_write_header(this->octx, 0);
    if (ret < 0) {
        printf("写入头信息失败 \n");
        return ret;
    }
    
    return 0;
}


