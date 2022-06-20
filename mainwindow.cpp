extern "C"{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <QTime>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    exit(0);
}

//Delay for some msec
void Delay(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < dieTime){
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
    }
}


void MainWindow::on_pushButton_clicked()
{

    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pCodecCtx = NULL;
    const AVCodec *pCodec = NULL;
    AVFrame *pFrame,*pFrameRGB;
    AVPacket *packet;
    struct SwsContext *img_convert_ctx;

    unsigned char *out_buffer;
    int i,videoIndex;
    int ret;
    char errors[1024] = "";

    //char filepath[] = "demo.avi";
    //rtsp地址:
    char url[] = "rtsp://admin:Comtom2021@192.168.111.60:554/LiveMedia/ch1/Media1";

    //初始化FFMPEG  调用了这个才能正常适用编码器和解码器
    pFormatCtx = avformat_alloc_context();  //init FormatContext
    //初始化FFmpeg网络模块
    avformat_network_init();    //init FFmpeg network


    //open Media File
    ret = avformat_open_input(&pFormatCtx,url,NULL,NULL);
    if(ret != 0){
        av_strerror(ret,errors,sizeof(errors));
        cout <<"Failed to open video: ["<< ret << "]"<< errors << endl;
        exit(ret);
    }

    //Get audio information
    ret = avformat_find_stream_info(pFormatCtx,NULL);
    if(ret != 0){
        av_strerror(ret,errors,sizeof(errors));
        cout <<"Failed to get audio info: ["<< ret << "]"<< errors << endl;
        exit(ret);
    }

    videoIndex = -1;

    ///循环查找视频中包含的流信息，直到找到视频类型的流
    ///便将其记录下来 videoIndex
    ///这里我们现在只处理视频流  音频流先不管他
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
        }
    }

    ///videoIndex-1 说明没有找到视频流
    if (videoIndex == -1) {
        printf("Didn't find a video stream.\n");
        return;
    }

    //配置编码上下文，AVCodecContext内容
    //1.查找解码器
    pCodec = avcodec_find_decoder(pFormatCtx->streams[videoIndex]->codecpar->codec_id);
    //2.初始化上下文
    pCodecCtx = avcodec_alloc_context3(pCodec);
    //3.配置上下文相关参数
    avcodec_parameters_to_context(pCodecCtx,pFormatCtx->streams[videoIndex]->codecpar);
    //4.打开解码器
    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if(ret != 0){
        av_strerror(ret,errors,sizeof(errors));
        cout <<"Failed to open Codec Context: ["<< ret << "]"<< errors << endl;
        exit(ret);
    }

    //初始化视频帧
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    //为out_buffer申请一段存储图像的内存空间
    out_buffer = (unsigned char*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32,pCodecCtx->width,pCodecCtx->height,1));
    //实现AVFrame中像素数据和Bitmap像素数据的关联
    av_image_fill_arrays(pFrameRGB->data,pFrameRGB->linesize, out_buffer,
                   AV_PIX_FMT_RGB32,pCodecCtx->width, pCodecCtx->height,1);
    //为AVPacket申请内存
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    //打印媒体信息
    av_dump_format(pFormatCtx,0,url,0);
    //初始化一个SwsContext
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);

    //设置视频label的宽高为视频宽高
    ui->label->setGeometry(0, 0, pCodecCtx->width, pCodecCtx->height);

    //读取帧数据，并通过av_read_frame的返回值确认是不是还有视频帧
    while(av_read_frame(pFormatCtx,packet) >=0){
        //判断视频帧
        if(packet->stream_index == videoIndex){
            //解码视频帧
            ret = avcodec_send_packet(pCodecCtx, packet);
            ret = avcodec_receive_frame(pCodecCtx, pFrame);
            if(ret != 0){
                av_strerror(ret,errors,sizeof(errors));
                cout <<"Failed to decode video frame: ["<< ret << "]"<< errors << endl;
            }
            if (ret == 0) {
                //处理图像数据
                sws_scale(img_convert_ctx,
                                        (const unsigned char* const*) pFrame->data,
                                        pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                                        pFrameRGB->linesize);
                QImage img((uchar*)pFrameRGB->data[0],pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
                ui->label->setPixmap(QPixmap::fromImage(img));
                //释放前需要一个延时
                Delay(1);
            }
        }
        //释放packet空间
        av_packet_unref(packet);
    }

    //close and release resource
    av_free(out_buffer);
    av_free(pFrameRGB);

    sws_freeContext(img_convert_ctx);
    avcodec_close(pCodecCtx);
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&pFormatCtx);
    exit(0);
}

