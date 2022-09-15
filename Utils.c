
#include<stdio.h>
#include<string.h>
#include<sys/stat.h>
#include<stdlib.h>
#include"Utils.h"
// uint32_t ReadUint32BE(char *data)
// {

// 	uint8_t *p = (uint8_t *)data;
// 	uint32_t value = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
// 	return value;
// }

// uint32_t ReadUint32LE(char *data)
// {
// 	uint8_t *p = (uint8_t *)data;
// 	uint32_t value = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
// 	return value;
// }

// uint32_t ReadUint24BE(char *data)
// {
// 	uint8_t *p = (uint8_t *)data;
// 	uint32_t value = (p[0] << 16) | (p[1] << 8) | p[2];
// 	return value;
// }

// uint32_t ReadUint24LE(char *data)
// {
// 	uint8_t *p = (uint8_t *)data;
// 	uint32_t value = (p[2] << 16) | (p[1] << 8) | p[0];
// 	return value;
// }

// uint16_t ReadUint16BE(char *data)
// {
// 	uint8_t *p = (uint8_t *)data;
// 	uint16_t value = (p[0] << 8) | p[1];
// 	return value;
// }

// uint16_t ReadUint16LE(char *data)
// {
// 	uint8_t *p = (uint8_t *)data;
// 	uint16_t value = (p[1] << 8) | p[0];
// 	return value;
// }
uint8_t* Load8(uint8_t *p, uint8_t x) {
    *p = x;
    return p+1;
}

uint8_t* Load16(uint8_t *p, uint16_t x) {
    p = Load8(p, (uint8_t)(x >> 8));
    p = Load8(p, (uint8_t)x);
    return p;
}

uint8_t* Load32(uint8_t *p, uint32_t x) {
    p = Load16(p, (uint16_t)(x >> 16));
    p = Load16(p, (uint16_t)x);
    return p;
}


int readFile(uint8_t **stream,int32_t* len,const uint8_t *filename){

	FILE *fp = NULL;
	long size =0;
	uint8_t *buff;

	fp = fopen(filename,"r");
	if(!fp)
		return -1;
	//读取文件的信息
	fseek(fp,0L,SEEK_END);
	size = ftell(fp);
	fseek(fp,0L,SEEK_SET);

	buff = (uint8_t*)malloc(sizeof(uint8_t)* size);
	memset(buff,0,(size_t)size);
	//第二个参数是元素的大小
	if(fread(buff,1,size,fp)!= size){

		printf("read error. \n");
		return -1;
	}

	fclose(fp);
	*stream = buff;
	*len = (int32_t)size;
	printf("file size is %d Bytes\n",*len);
	return 0;
}

