
#include<stdio.h>
#include<string.h>
#include"Net.h"

int udpInit(UDPContext *udp){

    if(udp == NULL || udp->dstIp == NULL || udp->dstPort == 0)
    {
        printf("udpInit error");
        return -1;
    }

    udp->socket = socket(AF_INET,SOCK_DGRAM,0);
    if(udp->socket <0){
        printf("Requested socket error");
        return -1;
    }

    udp->servAddr.sin_family = AF_INET;
    udp->servAddr.sin_port = htons(udp->dstPort);
    
    inet_aton(udp->dstIp,&udp->servAddr.sin_addr);
    return 0;
}


int udpSend(const UDPContext* udp,const uint8_t* buff,uint32_t len){

    ssize_t num = sendto(udp->socket,buff,len,0,(struct socketaddr*)&udp->servAddr,sizeof(udp->servAddr));
    if(num != len){
        //打印错误函数名
        printf("%s sendto error.%d%d\n",__FUNCTION__,(uint32_t)num,len);
        return -1;
    }
    return len;
}