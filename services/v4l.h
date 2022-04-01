#ifndef V4L_H
#define V4L_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/mman.h>
#include <getopt.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>
#include <linux/videodev2.h>


#define DEV_NAME "/dev/video0"      //摄像头设备名
#define DEV_NAME1 "/dev/video1"      //摄像头设备名(备用)

#define IMAGE_WIDTH 320//640//视频图像宽度
#define IMAGE_HEIGHT 240//480//视频图像高度
#define CLEAR(x) memset(&(x), 0, sizeof(x)) //空间清零

#define CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

struct buffer {
        void *                  start;
        size_t                  length;
};

class V4L
{
public:
    V4L();
    int videofd;            //摄像头文件描述符
    unsigned int fileLength; //视频图像的大小
    unsigned int nbuffers;
    struct buffer *buffers;
    struct v4l2_buffer tV4L2buf;
    unsigned  char frame_buffer[IMAGE_WIDTH*IMAGE_HEIGHT*3];

    int openVideo(const char *dev_name);
    void closeVideo(int fd);
    void getVideoFmt();
    int queryVideoCap();
    int setVideoFmt();
    int requestVideoBufsAndMmap();
    int readFrame();
    int stopCapture();
    int uninitVideo();
    int storeImage();
    unsigned char *encode_jpeg(unsigned char *lpbuf);
    int yuyv_2_rgb888(unsigned char *buffers, unsigned char *frame_buffer);
};

#endif // V4L_H
