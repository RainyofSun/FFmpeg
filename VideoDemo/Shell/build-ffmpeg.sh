#!/bin/sh
# component options:组件选项
#--disable-avdevice 禁止 libavcodec 构造
#--disable-avcodec 禁止 libavcore 构造
#--disable-avformat 禁止 libavformat 构造
#--disable-swresample 禁止 libswresample 构造
#--disable-swscale 禁止libswscale构造
#--disable-postproc 禁止libpostproc构造
#--disable-avfilter 禁止libavfilter 构造
#--enable-avresample 开启libavresample构造(默认关闭)
#--disable-pthreads 禁用 pthreads [默认：自动]
#--disable-w32threads 禁用 Win32 线程[默认：自动]
#--disable-os2threads 禁用0S/2线程[默认:自动]
#--disable-network 禁用网络支持[默认:关闭]
#--disable-dct 禁用DCT代码
#--disable-dwt 禁用DWT代码
#--disable-error-resilience 禁用错误resilience 代码
#--disable-lsp 禁用LSP代码
#--disable-lzo 禁用LZO代码
#--disable-mdct 禁用MDCT代码
#--disable-rdft 禁用RDFT代码
#--disable-fft 禁用FFT代码
#--disable-faan 禁用浮点 AAN(I)DCT代码
#Individual component options:个人组件选项
#--disable-everything 禁止下列所有组件
#--disable-encoder=NAME 禁止NAME的编码器
#--enable-encoder=NAME 启用NAME的编码器
#--disable-encoders 禁用所有编码器
#--disable-decoder=NAME 禁用NAME的解码器
#--enable-decoder=NAME 启用NAME的解码器
#--disable-decoders 禁用所有的解码器
#--disable-hwaccel=NAME 禁用NAME的硬件加速计
#--enable-hwaccel=NAME 启用NAME的硬件加速计
#--disable-muxer=NAME 禁用NAME的复用器
#--enable-muxer=NAME 启用NAME的复用器
#--disable-muxers 禁用所有的复用器
#--disable-demuxer=NAME 禁用NAME的分离器
#--enable-demuxer=NAME 启用NAME的分离器
#--disable-demuxers 禁用所有的分离器
#--enable-parser=NAME 启用NAME的解析器
#--disable-parser=NAME 禁用NAME的解析器
#--disable-parsers 禁用所有的解析器
#--enable-bsf=NAME 启用NAME的比特流过滤器
#--disable-bsf=NAME 禁用NAME的比特流过滤器
#--disable-bsfs 禁用所有的比特流过滤器
#--enable-protocol=NAME 启用NAME的协议
#--disable-protocol=NAME 禁用NAME的协议
#--disable-protocols 禁用所有的协议
#--enable-indev=NAME 启用NAME的输入设备
#--disable-indev=NAME 禁用NAME的输入设备
#--disable-indevs 禁用所有输入设备
#--enable-outdev=NAME 启用NAME的输出设备
#--disable-outdev=NAME 禁用NAME的输出设备
#--disable-devices 禁用所有的输出设备
#--enable-filter=NAME 启用NAME的过滤器
#--disable-filter=NAME 禁用NAME的过滤器
#--disable-filters 禁用所有的过滤器
#--disable-v412_m2m 禁用V4L2 mem2mem代码 [自动]
#External library support：外部库支持(重点)
#--disable-alsa 禁止ALSA支持[自动检测]
#--disable-appkit 禁止苹果AppKit框架[自动检测]
#--disable-avfoundation 禁止苹果AVFoundation框架[自动检测]
#--disable-avisynth 启用读取AviSynth脚本文件[默认关闭]
#--disable-bzlib 禁用 bzlib [自动检测]
#--disable-coreimage 禁用苹果CoreImage框架[自动检测]
#--enable-chromaprint Chromaprint：可提供一个客户端的公共库，能够通过特定算法计算音频文件的指纹，是AcoustID项目的核心。[默认关闭]
#--enable-freior 启用freior视频过滤[默认关闭]
#--enable-gcrypt 启用gcrypt,需要rtmp(t)e支持,如果openssl,librtmp,gmp没有被使用[默认关闭]
#--enable-gmp 启用gmp,需要rtmp(t)e支持,如果openssl或者librtmp没有被使用[默认关闭]
#--enable-gnutls 启用gnutls,需要https支持,如果opensll不能使用[默认关闭]
#--disable-iconv 禁用iconv[自动检测]
#--disable-jack 禁用libjack支持[自动检测]
#--enable-jni 启用JNI支持[默认关闭]
#--enable-ladspa 启用LADSPA 视频过滤[默认关闭]
#--enable-libass 启用libass字幕渲染,需要字幕和ass过滤[默认关闭]
#--enable-libbluray 启用蓝光,使用libbluray [默认关闭]
#--enable-libbs2b 启用bs2b DSP库[默认关闭]
#--enable-libcaca 启用文本显示,使用libcaca[默认关闭]
#--enable-libcelt 启用CELT通过解码libcelt[默认关闭]
#--enable-libcdio 启用视频CD,使用libcdio[默认关闭]
#--enable-libdc1394 启用 IIDC-1394 ,使用libdc1394和libraw1394[默认关闭]
#--enable-libfdk-aac 启用AAC 编码/解码通过libfdk-aac[默认关闭]
#--enable-libflite 启用过滤器支持通过libflite
#--enable-libfontconfig 启用libfontconfig,用于drawtext过滤器[默认关闭]
#--enable-libfreetype 启用libfreetype,需要darwtext过滤器[默认关闭]
#--enable-libfribidi 启用libfribidi ,优化darwtext过滤器[默认关闭]
#--enable-libgme 启用游戏音乐通过libgme[默认关闭]
#--enable-libgsm 启用GSM 编码/解码 通过libgsm[默认关闭]
#--enable-libiec61883 启用 iec61883 通过使用libiec61883[默认关闭]
#--enable-libilbc 启用iLBC 编码/解码通过libilbc[默认关闭]
#--enable-libkvazaar 启用HEVC 编码通过使用libkvazaar[默认关闭]
#--enable-libmodplug 启用ModPlug 通过libmodplug[默认关闭]
#--enable-libmp3lame 启用MP3编码通过libmp3lame[默认关闭]
#--enable-libopencore-amrnb 启用AMR-NB 编码/解码通过使用libopencore-amrnb[默认关闭]
#--enable-libopencore-amrwb 启用AMR-WB编码通过libopencore-amrwb[默认关闭]
#--enable-libopnecv 启用视频过滤器通过使用libopencv[默认关闭]
#--enable-libopenh264 启用H.264编码通过OpenH264[默认关闭]
#--enable-libopenjpeg 启用 JPEG 2000 编码/解码通过OpenJPEG[默认关闭]
#--enable-libopenmpt 启用编码追踪文件通过libopenmpt[默认关闭]
#--enable-libopus 启用Opus 编码/解码通过libpulse[默认关闭]
#--enable-libpulse 启用音频脉冲输入通过使用libpulse[默认关闭]
#--enable-librsvg 启用SVG 栅格化通过使用librsvg[默认关闭]
#--enable-librubberband 启用拖框需要拖框过滤器[默认关闭]
#--enable-librtmp 启用RTMP[E]支持通过使用librtmp[默认关闭]
#--enable-libshine 启用固定点MP3编码通过使用librsvg[默认关闭]
#--enable-libsmbclient 启用Samba协议通过使用libsmbclient[默认关闭]
#--enable-libsnappy 启用Snappy压缩,需要hap编码[默认关闭]
#--enable-libsoxr 启用libsoxr重采样[默认关闭]
#--enable-libspeex 启用speex 编码/解码通过使用libspeex[默认关闭]
#--enable-libssh 启用SFTP协议通过使用libspeex[默认关闭]
#--enable-libtesseract 启用Tesseract,需要ocr过滤器[默认关闭]
#--enable-libtheora 启用Theora编码通过使用libtheora[默认关闭]
#--enable-libtwolame 启用MP2编码通过使用libtwolame[默认关闭]
#--enable-lib412 启用libv412/v4l-utils[默认关闭]
#--enable-libvidstab 启用基于视频图像稳定,通过使用vid.stab[默认关闭]
#--enable-libvmaf 启用vmaf过滤器通过使用libvmaf[默认关闭]
#--enable-libvo-amrwbenc 启用AMR-WB编码通过使用libvo-amrwbenc[默认关闭]
#--enable-libvorbis 启用Vorbis编码/解码通过libvorbis,本地已经实现[默认关闭]
#--enable-libvpx 启用VP8和VP9编码/解码通过使用libvpx[默认关闭]
#--enable-libwavpack 启用wavpack编码通过使用libwavpack[默认关闭]
#--enable-libwebp 启用WebP编码通过使用libwebp[默认关闭]
#--enable-libx264 启用H.264编码通过使用x264[默认关闭]
#--enable-libx265 启用HEVC编码通过使用x265[默认关闭]
#--enable-libxavs 启用AVS编码通过使用xavs[默认关闭]
#--enable-libxcb 启用X11抓取XCB[自动检测]
#--enable-libxcb-shm 启用X11抓取shm交互[自动检测]
#--enable-libxcb-xfixes 启用X11抓取mouse渲染[自动检测]
#--enable-libxcb-shape 启用X11抓取图像渲染[自动检测]
#--enable-libxvid 启用Xvid编码通过使用xvidcore,本地MPEG-4/Xvid编码已经存在[默认关闭]
#--enable-libxml2 启用XML解析,使用C语言库libxml2[默认关闭]
#--enable-libzimg 启用z.lib 需要zscale过滤器[默认关闭]
#--enable-libzmq 启用消息传递通过使用libzmq[默认关闭]
#--enable-libzvbi 启用点在文本支持通过使用llibzvbi[默认关闭]
#--disable-lzma 禁用lzma[自动检测]
#--enable-decklink 启用黑魔法 DeckLind I/O支持[默认关闭]
#--enable-libndi_newtek 启用Newteck NDI I/O支持[默认关闭]
#--enable-mediacodec 启用安卓媒体编解码器支持[默认关闭]
#--enable-libmysofa 启用libmysofa,需要sofalizer过滤器[默认关闭]
#--enable-openal 启用OpenAL 1.1 捕获支持[默认关闭]
#--enable-onencl 启用OpenCl代码
#--enable-opengl 启用OpenGL渲染[默认关闭]
#--enable-openssl 启用openssl,需要https支持,如果没有使用[默认关闭]
#--disable-sndio 禁用sndio支持[自动检测]
#--disable-schannel 禁用 schannel SSP,需要TLS支持在Windows,如果openssl 和gnutls没有使用[自动检测]
#--disable-sdl2 禁用sdl2[自动检测]
#--disable-securetransport 禁用 安全传输,需要TLS支持在OSX如果openssl和gnutls没有使用[自动检测]
#--disable-xlib 禁用xlib[自动检测]
#--disable-zlib 禁用zlib[自动检测]
#--disable-audiotoolbox 禁用苹果AUdioToolbox代码[自动检测]
#--disable-cuda 禁用动态链接Nvidia CUDA代码[自动检测]
#--enable-cuda-sdk 启用CUDA特性,需要CUDA SDK[自动检测]
#--disable-cuvid 禁用 Nvidia CUVID 支持[自动检测]
#--disable-d3d11va 禁用Microsof Direct3D 11 视频加速器代码[自动检测]
#--disable-dxva2 禁用Microsof DirectX 9 视频加速器代码[自动检测]
#--enable-libdrm 启用DRM代码(Linux)[默认关闭]
#--enable-libmfx 启用Intel MediaSDK代码通过时会用libmfx[默认关闭]
#--enable-libnpp 启用Nvidia性能基本代码[默认关闭]
#--enable-mmal 启用多媒体抽象层通过使用MMAL[默认关闭]
#--disable-nvenc 禁用Nvidi视频编码代码[自动检测]
#--enable-omx 启用OpenMAX IL 代码[默认关闭]
#--enable-omx-rpi 启用OpenMAX IL 代码树莓派[默认关闭]
#--enable-rkmpp 启用媒体平台代码[默认关闭]
#--disable-vaapi 禁用视频加速器API代码[自动检测]
#--disable-vda 禁用苹果视频解码加速器代码[自动检测]
#--disable-vdpau 禁用Nvidia视频解码和展示Unix代码的API[自动检测]
#--disable-videotoolbox 禁用VideoToolbox代码[自动检测]

# directories
FF_VERSION="4.2"
#FF_VERSION="snapshot-git"
if [[ $FFMPEG_VERSION != "" ]]; then
  FF_VERSION=$FFMPEG_VERSION
fi
SOURCE="ffmpeg-$FF_VERSION"
FAT="FFmpeg-iOS"

SCRATCH="scratch"
# must be an absolute path
THIN=`pwd`/"thin"

# absolute path to x264 library
X264=`pwd`/fat-x264

FDK_AAC=`pwd`/fdk-aac-ios

CONFIGURE_FLAGS="   --enable-cross-compile \
                    --disable-debug \
                    --disable-programs \
                    --disable-doc \
                    --disable-stripping\
                    --disable-ffmpeg\
                    --disable-ffplay\
                    --disable-ffprobe\
                    --disable-asm\
                    --disable-yasm\
                    --enable-small\
                    --enable-dct\
                    --enable-dwt\
                    --enable-lsp\
                    --enable-mdct\
                    --enable-rdft\
                    --enable-fft\
                    --enable-version3\
                    --disable-filters\
                    --disable-postproc\
                    --disable-bsfs\
                    --enable-bsf=aac_adtstoasc\
                    --enable-bsf=h264_mp4toannexb\
                    --disable-encoders\
                    --enable-encoder=pcm_s161e\
                    --enable-encoder=aac\
                    --enable-encoder=libvo_aacenc\
                    --disable-decoders\
                    --enable-decoder=aac\
                    --enable-decoder=mp3\
                    --enable-decoder=pcm_s161e\
                    --disable-parsers\
                    --enable-parser=aac\
                    --disable-muxers\
                    --enable-muxer=mp4\
                    --enable-muxer=adts\
                    --enable-muxer=h264\
                    --disable-demuxers\
                    --enable-demuxer=mp4\
                    --enable-demuxer=aac\
                    --enable-demuxer=h264\
                    --disable-protocols\
                    --enable-protocol=rtmp\
                    --enable-protocol=file\
                    --enable-pic\
                    --enable-videotoolbox\
                    --enable-audiotoolbox\
                    --enable-encoder=h264_videotoolbox\
                    --enable-encoder=aac_at\
                    --enable-encoder=alac_at\
                    --enable-swscale\
                    --enable-avdevice\
                    "

if [ "$X264" ]
then
	CONFIGURE_FLAGS="$CONFIGURE_FLAGS --enable-gpl --enable-libx264"
fi

if [ "$FDK_AAC" ]
then
	CONFIGURE_FLAGS="$CONFIGURE_FLAGS --enable-libfdk-aac --enable-nonfree"
fi

# avresample
#CONFIGURE_FLAGS="$CONFIGURE_FLAGS --enable-avresample"

ARCHS="arm64 armv7 x86_64 i386"

COMPILE="y"
LIPO="y"

DEPLOYMENT_TARGET="8.0"

if [ "$*" ]
then
	if [ "$*" = "lipo" ]
	then
		# skip compile
		COMPILE=
	else
		ARCHS="$*"
		if [ $# -eq 1 ]
		then
			# skip lipo
			LIPO=
		fi
	fi
fi

if [ "$COMPILE" ]
then
	if [ ! `which yasm` ]
	then
		echo 'Yasm not found'
		if [ ! `which brew` ]
		then
			echo 'Homebrew not found. Trying to install...'
                        ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" \
				|| exit 1
		fi
		echo 'Trying to install Yasm...'
		brew install yasm || exit 1
	fi
	if [ ! `which gas-preprocessor.pl` ]
	then
		echo 'gas-preprocessor.pl not found. Trying to install...'
		(curl -L https://github.com/libav/gas-preprocessor/raw/master/gas-preprocessor.pl \
			-o /usr/local/bin/gas-preprocessor.pl \
			&& chmod +x /usr/local/bin/gas-preprocessor.pl) \
			|| exit 1
	fi

	if [ ! -r $SOURCE ]
	then
		echo 'FFmpeg source not found. Trying to download...'
		curl http://www.ffmpeg.org/releases/$SOURCE.tar.bz2 | tar xj \
			|| exit 1
	fi

	CWD=`pwd`
	for ARCH in $ARCHS
	do
		echo "building $ARCH..."
		mkdir -p "$SCRATCH/$ARCH"
		cd "$SCRATCH/$ARCH"

		CFLAGS="-arch $ARCH"
		if [ "$ARCH" = "i386" -o "$ARCH" = "x86_64" ]
		then
		    PLATFORM="iPhoneSimulator"
		    CFLAGS="$CFLAGS -mios-simulator-version-min=$DEPLOYMENT_TARGET"
		else
		    PLATFORM="iPhoneOS"
		    CFLAGS="$CFLAGS -mios-version-min=$DEPLOYMENT_TARGET -fembed-bitcode"
		    if [ "$ARCH" = "arm64" ]
		    then
		        EXPORT="GASPP_FIX_XCODE5=1"
		    fi
		fi

		XCRUN_SDK=`echo $PLATFORM | tr '[:upper:]' '[:lower:]'`
		CC="xcrun -sdk $XCRUN_SDK clang"

		# force "configure" to use "gas-preprocessor.pl" (FFmpeg 3.3)
		if [ "$ARCH" = "arm64" ]
		then
		    AS="gas-preprocessor.pl -arch aarch64 -- $CC"
		else
		    AS="gas-preprocessor.pl -- $CC"
		fi

		CXXFLAGS="$CFLAGS"
		LDFLAGS="$CFLAGS"
		if [ "$X264" ]
		then
			CFLAGS="$CFLAGS -I$X264/include"
			LDFLAGS="$LDFLAGS -L$X264/lib"
		fi
		if [ "$FDK_AAC" ]
		then
			CFLAGS="$CFLAGS -I$FDK_AAC/include"
			LDFLAGS="$LDFLAGS -L$FDK_AAC/lib"
		fi

		TMPDIR=${TMPDIR/%\/} $CWD/$SOURCE/configure \
		    --target-os=darwin \
		    --arch=$ARCH \
		    --cc="$CC" \
		    --as="$AS" \
		    $CONFIGURE_FLAGS \
		    --extra-cflags="$CFLAGS" \
		    --extra-ldflags="$LDFLAGS" \
		    --prefix="$THIN/$ARCH" \
		|| exit 1

		make -j3 install $EXPORT || exit 1
		cd $CWD
	done
fi

if [ "$LIPO" ]
then
	echo "building fat binaries..."
	mkdir -p $FAT/lib
	set - $ARCHS
	CWD=`pwd`
	cd $THIN/$1/lib
	for LIB in *.a
	do
		cd $CWD
		echo lipo -create `find $THIN -name $LIB` -output $FAT/lib/$LIB 1>&2
		lipo -create `find $THIN -name $LIB` -output $FAT/lib/$LIB || exit 1
	done

	cd $CWD
	cp -rf $THIN/$1/include $FAT
fi

echo Done
