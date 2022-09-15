#include<stdio.h>
#include<stdlib.h>

#include"Utils.h"
#include"RTP.h"
#include"Net.h"

int main(){

    int len = 0;
    int res ;
    uint8_t *stream = NULL;
    const char *filename = "Sample.h264";
    RTPMuxContext rtpMutContext;
    UDPContext udpCtx = {
        .dstIp = "127.0.0.1",
        .dstPort = 1234
    };

    res = readFile(&stream,&len,filename);
    if(res){
        printf("readfile error\n");
        return -1;
    }

    res = udpInit(&udpCtx);
    if(res){
        printf("udpInit error\n");
        return -1;
    }

    initRTPMutContext(&rtpMutContext);
    rtpSendH264HEVC(&rtpMutContext,&udpCtx,stream,len);

    free(stream);
    return 0;
}