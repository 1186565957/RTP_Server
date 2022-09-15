#include"Net.h"

#define RTP_MAX_LOAD 1400

typedef struct 
{
    uint8_t cache[RTP_MAX_LOAD+12];
    uint8_t buff[RTP_MAX_LOAD];
    uint8_t *buff_ptr;

    int aggregation;    //聚合包和单个包的标志为
    int Carrier_type;   //搭载数据的类型

    uint32_t ssrc;
    uint32_t seqment;
    uint32_t timestamp;
    
} RTPMuxContext;

int initRTPMutContext(RTPMuxContext *ctx);

void rtpSendH264HEVC(RTPMuxContext *ctx,UDPContext *udp,const uint8_t *buff,int size);
