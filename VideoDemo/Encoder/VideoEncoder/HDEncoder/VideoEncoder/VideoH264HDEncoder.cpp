//
//  VideoH264HDEncoder.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/11.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "VideoH264HDEncoder.hpp"

bool VideoH264HDEncoder::initX264Encoder(int width, int height, int videoBitRate, int frameRate,const char *videoFilePath) {
    
    frameCounter    = 0;
    frameWidth      = width;
    frameHeight     = height;
    this->videoFilePath = videoFilePath;
    this->writeHeaderSeccess = true;
    this->encode_result = 0;
    this->isNeedWriteLocal = strlen(videoFilePath) != 0;
    
    int ret = -1;
    ret = this->initializationFormat();
    if (ret < 0) {
        return false;
    }
    
    ret = this->initializationCodexCtx(width, height, frameRate, videoBitRate);
    if (ret < 0) {
        printf("alloc video stream failed");
        return false;
    }
    
    this->initializationAVFrame();
    return true;
}

// 开始编码
void VideoH264HDEncoder::encode(I420Buffer buffer, void *(*VideoEncodeCallBack)(AVPacket *)) {
    // YUV data
    pFrame->data[0] = buffer.y_frame;   // Y
    pFrame->data[1] = buffer.u_frame;   // U
    pFrame->data[2] = buffer.v_frame;   // V
    // PTS/时间戳
    pFrame->pts = frameCounter;
    
    // 发送一帧视频像素数据
    avcodec_send_frame(pCodecCtx, pFrame);
    // 接收一帧视频像素数据--编码--视频压缩数据格式
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    memset(packet, 0, sizeof(AVPacket));
    // 接收一帧视频像素数据--编码--视频压缩数据格式
    this->encode_result = avcodec_receive_packet(pCodecCtx, packet);
    if (this->encode_result == 0) {
        // 编码成功
        // 时间戳
        frameCounter ++;
        // 将视频压缩数据写入输出文件中
        packet->stream_index = video_stream->index;
//        packet->pts = frameCounter;
        // 非压缩时候的数据（即YUV或者其它），在ffmpeg中对应的结构体为AVFrame,它的时间基为AVCodecContext 的time_base
        // 压缩后的数据（对应的结构体为AVPacket）对应的时间基为AVStream的time_base
        // 用于将AVPacket中各种时间值从一种时间基转换为另一种时间基
        av_packet_rescale_ts(packet, pCodecCtx->time_base, video_stream->time_base);
        packet->pos = -1;
        printf("当前编码到第 %d帧  video pts = %lld\n",frameCounter,packet->pts);
        // 编码数据返回上层处理
        VideoEncodeCallBack(packet);
        if (this->isNeedWriteLocal) {
            this->encode_result = av_write_frame(avFormatCtx, packet);
//            printf("Successed to encode frame: %5d\tsize:%5d\n",frameCounter,packet->size);
            if (this->encode_result < 0) {
                printf("输出一帧数据失败\n");
                return;
            }
        }
    }
}

// 销毁编码器
void VideoH264HDEncoder::freeEncoder() {
    printf("开始析构\n");
//    this->encode_result = this->flush_encoder();
//    if (this->encode_result < 0) {
//        printf("Flushing encoder failed\n");
//        return;
//    }
    
    if (this->isNeedWriteLocal) {
        // 写文件尾
        if (this->writeHeaderSeccess) {
            av_write_trailer(avFormatCtx);
        }
    }
    
    avcodec_close(pCodecCtx);
    av_free(pFrame);
    avio_close(avFormatCtx->pb);
    avformat_free_context(avFormatCtx);
    pCodecCtx = NULL;
    pFrame = NULL;
    printf("video Codec Dealloc\n");
}

#pragma mark - private methods
// 初始化封装格式
int VideoH264HDEncoder::initializationFormat() {
    avcodec_register_all();
    int ret = 0;
    // 初始化封装格式上下文--视频编码--处理为视频数据格式
    avFormatCtx = avformat_alloc_context();
    if (this->isNeedWriteLocal) {
        // 注意事项：FFmepg程序推测输出文件类型->视频压缩数据格式类型
        // 得到视频压缩数据格式类型(h264、h265、mpeg2等等...)
        avOutputCtx = av_guess_format(NULL, this->videoFilePath, NULL);
        // 指定类型
        avFormatCtx->oformat = avOutputCtx;
        ret = this->openOutputFile();
        if (ret < 0) {
            return ret;
        }
    }
    return ret;
}

// 打开输出文件
int VideoH264HDEncoder::openOutputFile() {
    int ret = 0;
    ret = avio_open(&avFormatCtx->pb, this->videoFilePath, AVIO_FLAG_WRITE);
    if (ret < 0) {
        printf("打开输出文件失败");
        return ret;
    }
    return ret;
}

// 初始化编码器上下文
int VideoH264HDEncoder::initializationCodexCtx(int width, int height,int frameRate,int videoBitRate) {
    int ret = -1;
    // 查找编码器
    pCodec = avcodec_find_encoder_by_name("h264_videotoolbox");
    pCodec->id = AV_CODEC_ID_H264;
    pCodec->type = AVMEDIA_TYPE_VIDEO;
    pCodec->pix_fmts = (AVPixelFormat *)AV_PIX_FMT_YUV420P;
    pCodec->channel_layouts = (uint64_t *)1;
    
    if (!pCodec) {
        printf("未能找到编码器");
        return ret;
    }
    // 创建输出码流
    video_stream = avformat_new_stream(avFormatCtx, pCodec);
    video_stream->time_base.den = frameRate;
    video_stream->time_base.num = 1;
    // 实际帧率
    video_stream->r_frame_rate.den = 1;
    video_stream->r_frame_rate.num = frameRate;
    video_stream->codecpar->codec_tag = 0;
    video_stream->start_time = 0;
    
    // 获取编码器上下文
    pCodecCtx = avcodec_alloc_context3(pCodec);
//    pCodecCtx = video_stream->codec;
    // 设置编码器ID
    pCodecCtx->codec_id = avOutputCtx->video_codec;
    // 设置编码器类型
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    // 设置读取像素格式--编码的像素数据格式--视频像素数据格式--YUV420P、YUV422P、YUV444P等
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    // 视频宽高
    pCodecCtx->width = frameWidth;
    pCodecCtx->height = frameHeight;
    // 设置帧率
    // f-->帧率：25:000 fps
    // ps表示：时间单位/秒
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = frameRate;
    // 设置码率
    // 什么是码率？
    // 含义：每秒传送的比特(bit)数单位为 bps(Bit Per Second)，比特率越高，传送数据速度越快。
    // 单位：bps，"b"表示数据量，"ps"表示每秒
    // 目的：视频处理->视频码率
    // 什么是视频码率?
    // 含义：视频码率就是数据传输时单位时间传送的数据位数，一般我们用的单位是kbps即千位每秒
    // 视频码率计算如下？
    // 基本的算法是：【码率】(kbps)=【视频大小 - 音频大小】(bit位) /【时间】(秒)
    // 例如：Test.mov时间 = 24，文件大小(视频+音频) = 1.73MB
    // 视频大小 = 1.34MB（文件占比：77%） = 1.34MB * 1024 * 1024 * 8 = 字节大小 = 468365字节 = 468Kbps
    // 音频大小 = 376KB（文件占比：21%）
    // 计算出来值->码率 : 468Kbps->表示1000，b表示位(bit->位)
    // 总结：码率越大，视频越大
    pCodecCtx->bit_rate = videoBitRate;
    // 设置GOP->影响到视频质量问题->画面组->一组连续画面
    // MPEG格式画面类型：3种类型->分为->I帧、P帧、B帧
    // I帧->内部编码帧->原始帧(原始视频数据)
    //    完整画面->关键帧(必需的有，如果没有I，那么你无法进行编码，解码)
    //    视频第1帧->视频序列中的第一个帧始终都是I帧，因为它是关键帧
    // P帧->向前预测帧->预测前面的一帧类型，处理数据(前面->I帧、B帧)
    //    P帧数据->根据前面的一帧数据->进行处理->得到了P帧
    // B帧->前后预测帧(双向预测帧)->前面一帧和后面一帧
    //    B帧压缩率高，但是对解码性能要求较高。
    // 总结：I只需要考虑自己 = 1帧，P帧考虑自己+前面一帧 = 2帧，B帧考虑自己+前后帧 = 3帧
    //    说白了->P帧和B帧是对I帧压缩
    // 每250帧，插入1个I帧，I帧越少，视频越小->默认值->视频不一样
    pCodecCtx->gop_size = (int)frameRate;
    // 设置b帧最大值->设置不需要B帧
    pCodecCtx->max_b_frames = 0;
    
    pCodecCtx->i_quant_factor = 0.8;
    // 设置量化参数->数学算法(高级算法)
    // 总结：量化系数越小，视频越是清晰
    // 一般情况下都是默认值，最小量化系数默认值是10，最大量化系数默认值是51
    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;
    pCodecCtx->me_range = 16;
    pCodecCtx->max_qdiff = 4;
    pCodecCtx->qcompress = 0.6;

    // 码率控制
    pCodecCtx->rc_min_rate = videoBitRate - delta * 1000;
    pCodecCtx->rc_max_rate = videoBitRate + delta * 1000;
    pCodecCtx->rc_buffer_size = videoBitRate * 2;
    if(pCodecCtx->flags & AVFMT_GLOBALHEADER){
        pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    //从编码器复制参数
    avcodec_parameters_from_context(video_stream->codecpar, pCodecCtx);
    avcodec_parameters_to_context(video_stream->codec, video_stream->codecpar);
    
    // h264 编码配置
    AVDictionary *params = NULL;
    if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
        // 编码速度值有ultrafast、superfast、veryfast、faster、fast、medium、slow、slower、veryslow、placebo，越快视频质量则越差
        av_dict_set(&params, "preset", "slow", 0);
        //第二个值：调优 key: tune->调优 value: zerolatency->零延迟
        av_dict_set(&params, "tune", "zerolatency", 0);
        av_dict_set(&params, "profile", "main", 0);
    }
    
    //Show some Information
    av_dump_format(avFormatCtx, 0, this->videoFilePath, 1);
    
    // 打开编码器
    ret = avcodec_open2(pCodecCtx, pCodec, &params);
    if (ret < 0) {
        printf("打开编码器失败");
        return ret;
    }
    
    if (this->isNeedWriteLocal) {
        // 写文件头
        if (avformat_write_header(avFormatCtx, NULL) < 0) {
            this->writeHeaderSeccess = false;
            printf("文件头写入失败");
            return ret;
        }
    }
    
    return ret;
}

void VideoH264HDEncoder::initializationAVFrame(void) {
    // 定义一个缓冲区 缓存一帧视频像素数据 获取缓存区大小
    pictureSize = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    // AVFrame开辟内存空间 时间基基于AVCodecContext
    pFrame = av_frame_alloc();
    pFrame->format = pCodecCtx->pix_fmt;
    pFrame->width = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;
    // 宽高比
    pFrame->sample_aspect_ratio.num = 16;
    pFrame->sample_aspect_ratio.den = 9;
    // 时间戳
    pFrame->pts = 0;
    // 颜色空间
    pFrame->color_range = AVCOL_RANGE_MPEG;
    pFrame->pkt_duration = 0;
    
    // 设置缓冲区和AVFrame类型保持一致
    avpicture_fill((AVPicture *)pFrame, NULL, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
//    av_image_fill_arrays(pFrame->data, pFrame->linesize, this->out_buffer, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 0);
}

// 冲洗编码器
int VideoH264HDEncoder::flush_encoder() {
    int ret,get_frame;
    AVPacket enc_pkt;
    if (!(avFormatCtx->streams[0]->codec->codec->capabilities & AV_CODEC_CAP_DELAY)) {
        return 0;
    }
    
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_audio2(avFormatCtx->streams[0]->codec, &enc_pkt, NULL, &get_frame);
        av_frame_free(NULL);
        
        if (ret < 0) {
            break;
        }
        if (!get_frame) {
            ret = 0;
            break;
        }
        
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        ret = av_write_frame(avFormatCtx, &enc_pkt);
        if (ret < 0) {
            break;
        }
    }
    return ret;
}
