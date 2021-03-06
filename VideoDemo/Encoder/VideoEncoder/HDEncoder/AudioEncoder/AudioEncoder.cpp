//
//  AudioEncoder.cpp
//  VideoDemo
//
//  Created by EGLS_BMAC on 2020/6/17.
//  Copyright © 2020 EGLS_BMAC. All rights reserved.
//

#include "AudioEncoder.hpp"

bool AudioEncoder::initAACEncoder(int sample_rate, int channels, int bit_rate, const char *aacFilePath) {
    return this->initAACEncoder(sample_rate, channels, bit_rate, 44100, aacFilePath);
}

bool AudioEncoder::initAACEncoder(int sample_rate, int channels, int bit_rate, int bitsPerSample, const char *aacFilePath) {
    
    this->aacFilePath = aacFilePath;
    this->sample_rate = sample_rate;
    this->bit_rate    = bit_rate;
    this->channels    = channels;
    this->samplesCursor = 0;
    this->ret         = 0;
    this->writeHeaderSeccess = true;
    this->isNeedWriteLocal = strlen(this->aacFilePath) != 0;
    
    int ret = this->malloc_audio_stream();
    
    return ret >= 0;
}

int AudioEncoder::aacEncode(uint8_t *buffer, int size, void *(*AudioEncoderCallBack)(AVPacket *)) {
    int bufferCursor = 0;
    int bufferSize = size;
    while (bufferSize >= (buffer_size - samplesCursor)) {
        int cpySize = buffer_size - samplesCursor;
        memcpy(pcm_samples + samplesCursor, buffer + bufferCursor, cpySize);
        bufferCursor += cpySize;
        bufferSize -= cpySize;
        // 编码sample数据
        this->audio_encode(AudioEncoderCallBack);
        samplesCursor = 0;
    }
    // 按照缓存区长度取数据，如若剩余部分的数据小于缓存区长度，如下处理
    if (bufferSize > 0) {
        memcpy(pcm_samples + samplesCursor, buffer + bufferCursor, bufferSize);
        samplesCursor += bufferSize;
    }
    
    return 1;
}

void AudioEncoder::freeAACEncode() {
    printf("开始析构\n");
//    this->ret = this->flush_encoder();
//    if (this->ret < 0) {
//        printf("Flushing encoder failed\n");
//        return;
//    }
    
    if (this->isNeedWriteLocal) {
        // 写文件尾
        if (this->writeHeaderSeccess) {
            av_write_trailer(avFormatCtx);
        }
    }
    
    avcodec_close(avAudioCtx);
    av_free(audio_stream);
    avio_close(avFormatCtx->pb);
    printf("audio Codec Dealloc\n");
}

#pragma mark - private methods
int AudioEncoder::malloc_audio_stream() {
    av_register_all();
    
    int ret = 0;
    // 创建封装格式的上下文
    avFormatCtx = avformat_alloc_context();
    
    if (this->isNeedWriteLocal) {
        // FFMpeg推测出输出文件类型--音频压缩数据格式类型--aac格式
        avOutputCtx = av_guess_format(NULL, this->aacFilePath, NULL);
        // 指定类型
        avFormatCtx->oformat = avOutputCtx;
        // 打开输出文件
        ret = this->openOutputFile();
        if (ret < 0) {
            return ret;
        }
    }
    
    // 查找、打开音频编码器
    ret = this->find_audio_codec();
    if (ret < 0) {
        return ret;
    }
    ret = this->malloc_audio_pFrame();
    if (ret < 0) {
        return ret;
    }
    return ret;
}

// 打开输出文件
int AudioEncoder::openOutputFile() {
    int ret = 0;
    ret = avio_open(&avFormatCtx->pb, this->aacFilePath, AVIO_FLAG_WRITE);
    if (ret < 0) {
        printf("打开输出文件失败");
        return ret;
    }
    return ret;
}

// 查找、打开音频编码器
int AudioEncoder::find_audio_codec() {
    // 查找音频编码器
    audio_codec = avcodec_find_encoder_by_name("aac_at");
    audio_codec->id = AV_CODEC_ID_AAC;
    audio_codec->type = AVMEDIA_TYPE_AUDIO;
    
    if (!audio_codec) {
        printf("未找到音频编码器");
        return -1;
    }
    
    // 创建输出码流--创建一块内存空间--这里不知道是什么类型的流
    audio_stream = avformat_new_stream(avFormatCtx, audio_codec);
    audio_stream->time_base.num = 1;
    audio_stream->time_base.den = sample_rate;
    // 实际帧率
    audio_stream->r_frame_rate.num = sample_rate;
    audio_stream->r_frame_rate.den = 1;
    
    // 获取编码器上下文
//    avAudioCtx = this->audio_stream->codec;
    avAudioCtx = avcodec_alloc_context3(audio_codec);
    // 设置编码器上下文参数
    avAudioCtx->codec_id = avOutputCtx->audio_codec;
    // 编码格式
    avAudioCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    // 设置读取音频采样格式
    avAudioCtx->sample_fmt = AV_SAMPLE_FMT_S16;
    // 采样率
    avAudioCtx->sample_rate = this->sample_rate;
    // 立体声
    avAudioCtx->channel_layout = this->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
    // 声道数量
    avAudioCtx->channels = av_get_channel_layout_nb_channels(avAudioCtx->channel_layout);
    // 设置码率 码率】(kbps)=【视频大小 - 音频大小】(bit位) /【时间】(秒)
    avAudioCtx->bit_rate = this->bit_rate;
    avAudioCtx->frame_size = 1024;
    avAudioCtx->time_base.num = 1;
    avAudioCtx->time_base.den = avAudioCtx->sample_rate;
    avAudioCtx->profile = FF_PROFILE_AAC_MAIN;
    avAudioCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    if(avAudioCtx->flags & AVFMT_GLOBALHEADER) {
        avAudioCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    // 打开音频编码器
    if (avcodec_open2(avAudioCtx, audio_codec, NULL) < 0) {
        printf("打开编码器失败");
        return -1;
    }
    
    //Show some Information
    av_dump_format(avFormatCtx, 0, this->aacFilePath, 1);
    
    return 0;
}

int AudioEncoder::malloc_audio_pFrame() {
    audio_frame = av_frame_alloc();
    audio_frame->nb_samples = avAudioCtx->frame_size;
    audio_frame->format = avAudioCtx->sample_fmt;
    audio_frame->channels = avAudioCtx->channels;
    audio_frame->channel_layout = avAudioCtx->channel_layout;
    audio_frame->sample_rate = avAudioCtx->sample_rate;
    
    // 得到音频采样数据缓冲区大小
    this->buffer_size = av_samples_get_buffer_size(NULL, av_get_channel_layout_nb_channels(audio_frame->channel_layout),audio_frame->nb_samples, (AVSampleFormat)audio_frame->format, 0);
    // 创建缓冲区--存储音频采样数据--一帧数据
    pcm_samples = (uint8_t *)av_malloc(this->buffer_size);
    if (!pcm_samples) {
        LOGI("Could not allocate %d bytes for samples buffer\n", buffer_size);
        return -2;
    }
    printf("创建缓冲一帧数据大小 %d\n",this->buffer_size);
    ret = avcodec_fill_audio_frame(audio_frame, av_get_channel_layout_nb_channels(audio_frame->channel_layout), (AVSampleFormat)audio_frame->format, this->pcm_samples, this->buffer_size, 0);
    if (ret < 0) {
        LOGI("Could not setup audio frame reason = %s\n",av_err2str(ret));
        return -1;
    }
    
    audio_stream->codecpar->codec_tag = 0;
    audio_stream->time_base = audio_stream->codec->time_base;
    //从编码器复制参数
    avcodec_parameters_from_context(audio_stream->codecpar, avAudioCtx);
    avcodec_parameters_to_context(audio_stream->codec, audio_stream->codecpar);
    
    if (this->isNeedWriteLocal) {
        // 写文件头
        if (avformat_write_header(avFormatCtx, NULL) < 0) {
            this->writeHeaderSeccess = false;
            printf("文件头写入失败");
            return -1;
        }
    }
    
    return 0;
}

// 编码一帧数据
int AudioEncoder::audio_encode(void *(*AudioEncoderCallBack)(AVPacket *)) {
    // 设置采样数据格式
    audio_frame->data[0] = this->pcm_samples;
    audio_frame->pts = frame_current * audio_frame->nb_samples;
    
    // 编码一帧音频采样数据--得到一帧音频压缩数据--aac
    // 发送一帧音频数据
    ret = avcodec_send_frame(avAudioCtx, audio_frame);
    if (ret != 0) {
        printf("failed to send Frame %s\n",av_err2str(ret));
        return ret;
    }
    // 创建音频帧压缩数据--帧缓存空间
    AVPacket *audio_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    // 编码一帧采样数据
    memset(audio_packet, 0, sizeof(AVPacket));
    ret = avcodec_receive_packet(avAudioCtx, audio_packet);
    if (ret == 0) {
        frame_current ++;
        // 编码后的音频流写入文件
        audio_packet->stream_index = audio_stream->index;
        audio_packet->duration = audio_packet->pts * av_q2d(this->audio_stream->time_base);
//        frame_current += av_rescale_q(audio_frame->nb_samples, {1,avAudioCtx->sample_rate}, avAudioCtx->time_base);// pts 自增1024 有疑问？
        // 非压缩时候的数据（即YUV或者其它），在ffmpeg中对应的结构体为AVFrame,它的时间基为AVCodecContext 的time_base
        // 压缩后的数据（对应的结构体为AVPacket）对应的时间基为AVStream的time_base
        // 用于将AVPacket中各种时间值从一种时间基转换为另一种时间基
        audio_packet->pts = av_rescale_q(audio_frame->pts, avAudioCtx->time_base, audio_stream->time_base);
//        av_packet_rescale_ts(audio_packet, avAudioCtx->time_base, audio_stream->time_base);
        audio_packet->dts = audio_packet->pts;
        printf("音频编码到第 %d帧 audio pts = %lld\n",frame_current,audio_packet->pts);
        audio_packet->pos = -1;
        // 编码数据返回上层处理
        AudioEncoderCallBack(audio_packet);
        if (this->isNeedWriteLocal) {
            ret = av_write_frame(avFormatCtx, audio_packet);
            if (ret < 0) {
                printf("编码数据写入失败\n");
                return ret;
            }
        }
    } else {
        printf("编码失败\n");
        return ret;
    }
    
    return ret;
}

// 冲洗编码器
int AudioEncoder::flush_encoder() {
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

