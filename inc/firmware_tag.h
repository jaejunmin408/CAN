#ifndef FIRMWARE_INFO_H
#define FIRMWARE_INFO_H

#include <stdint.h>

typedef struct {
    uint32_t size;
    uint32_t crc;
    uint32_t version;
    uint32_t valid_flag;
    char filename[32];
} FirmwareTag;

extern FirmwareTag firmware_tag;

#endif
