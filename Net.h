#pragma once
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

typedef struct {

    const char* dstIp;
    int dstPort;
    struct sockaddr_in servAddr;
    int socket;
}UDPContext;

int udpInit(UDPContext *udp);

int udpSend(const UDPContext* udp,const uint8_t* buff,uint32_t len);



