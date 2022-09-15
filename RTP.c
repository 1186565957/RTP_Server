/*
**总体的次序和方式：
    找到开始的其实码，两个起始码之间进行包的发送情况。
    先是构建RTP包的整体头格式，每一个发出去的数据都会携带这个包头

    这个是聚合：构建H264的格式体，构建NAL头文件 NALU长度 NALU的内容
    //不使用聚合包，直接一次性把所有的数据都传输走
    分片的方式：FU Indicator也就是上面的NAL头格式，FU Header头 数据

    需要查看是否超过了RTP的承载量，超过了就发送除去，将剩余的部分划分为
    end报端

**   
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "RTP.h"
#include "Utils.h"
#include "AVC.h"

#define RTP_VERSION 2
#define RTP_H264 96

static UDPContext *g_udpContext;

int initRTPMutContext(RTPMuxContext *ctx)
{
    ctx->seqment = 0;
    ctx->timestamp = 0;
    ctx->ssrc = 0x35478458;
    ctx->aggregation = 1;
    ctx->buff_ptr = ctx->buff;
    ctx->Carrier_type = 0; //代表h264
    return 0;
}

void rtpSendData(RTPMuxContext *ctx, const uint8_t *buff, int len, int mark)
{

    /*
     *
     *    0                   1                   2                   3
     *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
     *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *   |                           timestamp                           |
     *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *   |           synchronization source (SSRC) identifier            |
     *   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
     *   |            contributing source (CSRC) identifiers             |
     *   :                             ....                              :
     *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     **/
    int res = 0; //返回值
    uint8_t *pos = ctx->cache;
    pos[0] = (RTP_VERSION << 6) & 0xff;
    //M是视频结束的标志，mark对应这M
    pos[1] = (uint8_t)((RTP_H264 & 0x7f)) | ((mark & 0x01) << 7); //M PT
    Load16(&pos[2], (uint16_t)ctx->seqment);
    Load32(&pos[4], ctx->timestamp);
    Load32(&pos[8], ctx->ssrc);
    //数据部分，里面有很大一部分的空间剩余
    memcpy(&pos[12], buff, len);

    res = udpSend(g_udpContext, ctx->cache, (uint32_t)(len + 12));
    printf("udpSend cache %d:", res);
    for (int i = 0; i < 20; ++i)
    {
        printf("%.2X ", ctx->cache[i]);
    }
    printf("\n");

    memset(ctx->cache, 0, RTP_MAX_LOAD + 10);
    ctx->buff_ptr = ctx->buff;
    ctx->seqment = (ctx->seqment + 1) & 0xffff;
}

static void rtpSendNAL(RTPMuxContext *ctx, const uint8_t *nal, int size, int last)
{

    //可以单个包发送时
    if (size <= RTP_MAX_LOAD)
    {
        //是否是聚合包发送方式
        if (ctx->aggregation)
        {

            int buffer_size = (int)(ctx->buff_ptr - ctx->buff);
            uint8_t curNRI = (uint8_t)(nal[0] & 0x60); //0110 0000 NAL NRI 序号用的是NAL包的首部

            //当前的生于空间小于总的需求空间，直接把当前的东西发出去
            //2的标志为代表头格式和长度的信息
            if (buffer_size + 2 + size > RTP_MAX_LOAD)
            {
                rtpSendData(ctx, ctx->buff, buffer_size, 0);
                buffer_size = 0;
            }

            /*
             *    STAP-A/AP NAL Header
             *     +---------------+
             *     |0|1|2|3|4|5|6|7|
             *     +-+-+-+-+-+-+-+-+
             *     |F|NRI|  Type   |
             *     +---------------+
             * */
            //第一次发送，里面是空的
            //构造一个STAP-A的头部
            if (buffer_size == 0)
            {
                //首字符的尾部的下一个元素，插入NRI的值
                *ctx->buff_ptr++ = (uint8_t)(24 | curNRI); //0x18  0001 1000
            }
            else
            { 
                //最后一个NRI是用开头与操作，这是能代表的最大的NRI，也就是3
                uint8_t lasetNRI = (uint8_t)(ctx->buff[0] & 0x60);            //0110 0000
                if (curNRI > lasetNRI)                                        //开始使用新的NRI
                    ctx->buff[0] = (uint8_t)((ctx->buff[0] & 0x9F) | curNRI); //1001 1111
            }

            // set STAP-A/AP NAL Header F = 1, if this NAL F is 1.
            ctx->buff[0] |= (nal[0] & 0x80); //1000 0000

            //NAL Size  NALU Header NALU Data
            Load16(ctx->buff_ptr, (uint16_t)size);
            ctx->buff_ptr += 2;
            //head and data
            memcpy(ctx->buff_ptr, nal, size);
            ctx->buff_ptr += size;

            //要组建最后一个NAL包的时候一起发送出去
            //标志位
            if (last == 1)
                rtpSendData(ctx, ctx->buff, (int)(ctx->buff_ptr - ctx->buff), 1);
        }
        else
        {
            /*
             *   0 1 2 3 4 5 6 7 8 9
             *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  |F|NRI|  Type   | a single NAL unit ... |
             *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             * */
            rtpSendData(ctx, nal, size, last);
        }
    }

    else
    { //分片的方式

        /*  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         * | FU indicator  |   FU header   |   FU payload   ...  |
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         * */
        //里面有数据了已经
        if (ctx->buff_ptr > ctx->buff)
            rtpSendData(ctx, ctx->buff, (int)(ctx->buff_ptr - ctx->buff), 0);

        int headerSize;
        uint8_t *buff = ctx->buff;
        uint8_t type = nal[0] & 0x1F; //0001 1111
        uint8_t NRI = nal[0] & 0x60;  //0110 0000

        /*
         *     FU Indicator
         *    0 1 2 3 4 5 6 7
         *   +-+-+-+-+-+-+-+-+
         *   |F|NRI|  Type   |
         *   +---------------+
         * */

        buff[0] = 28; //代表Fu-A 0001 1100
        buff[0] |= NRI;

        /*
         *      FU Header
         *    0 1 2 3 4 5 6 7
         *   +-+-+-+-+-+-+-+-+
         *   |S|E|R|  Type   |
         *   +---------------+
         * */

        buff[1] = type;    //和NALU header相同
        buff[1] |= 1 << 7; //S = 1;
        headerSize = 2;
        size -= 1;
        nal += 1;

        //过大了需要发送，之后设置为结尾报段
        while (size + headerSize > RTP_MAX_LOAD)
        {

            memcpy(&buff[headerSize], nal, (size_t)(RTP_MAX_LOAD - headerSize));
            rtpSendData(ctx, buff, RTP_MAX_LOAD, 0);
            nal += RTP_MAX_LOAD - headerSize;
            size -= RTP_MAX_LOAD - headerSize;
            buff[1] &= 0x7f; //0111 1111
        }

        buff[1] |= 0x40; //0100 0000
        memcpy(&buff[headerSize], nal, size);
        rtpSendData(ctx, buff, size + headerSize, last);
    }
}

void rtpSendH264HEVC(RTPMuxContext *ctx, UDPContext *udp, const uint8_t *buff, int size)
{

    const uint8_t *startCode;
    const uint8_t *end = buff + size;
    g_udpContext = udp;

    printf("UdpSend Start");

    if (ctx == NULL || udp == NULL || buff == NULL || size <= 0)
    {
        printf("rtpSendH264 paramse error");
        return;
    }

    startCode = ffmage_avc_find_startcode(buff, end);
    while (startCode < end)
    {

        const uint8_t *ptr;
        while (!*(startCode++))
            ; //跳过开始的字符
        ptr = ffmage_avc_find_startcode(startCode, end);

        rtpSendNAL(ctx, startCode, (int)(ptr - startCode), ptr == end);

        usleep(1000000 / 25);

        ctx->timestamp += (90000.0 / 25);
        startCode = ptr;
    }
}