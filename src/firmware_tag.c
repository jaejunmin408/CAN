#include "firmware_tag.h"
#include "main.h"

FirmwareTag __attribute__((section(".firmware_tag"))) firmware_tag = {
    .size = 0x0010000,
    .crc = 0xFFFFFFFF,
    .version = 0x00010001,
    .valid_flag = 0xA5A5A5A5,
    .filename = "webserver.bin"
};
