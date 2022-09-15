#include <stdint.h>
//本质上是针对大字节和小字节序列的移动问题
// uint32_t ReadUint32BE(char* data);
// uint32_t ReadUint32LE(char* data);

// uint32_t ReadUint24BE(char* data);
// uint32_t ReadUint24LE(char* data);

// uint16_t ReadUint16BE(char* data);
// uint16_t ReadUint16LE(char* data);

// uint8_t ReadUint8(char* data);

uint8_t *Load8(uint8_t *p, uint8_t x);

uint8_t *Load16(uint8_t *p, uint16_t x);

uint8_t *Load32(uint8_t *p, uint32_t x);

int readFile(uint8_t **stream, int32_t* len, const uint8_t *filename);

