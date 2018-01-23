#ifndef DANAOS_BOOTLOADER_DEVICES_CGA_H_
#define DANAOS_BOOTLOADER_DEVICES_CGA_H_

#include <stdint.h>

void cga_clear();
void print(const char *msg);
void print_int(int32_t val, unsigned char base);
void print_uint(uint32_t val, unsigned char base);

#endif //DANAOS_BOOTLOADER_DEVICES_CGA_H_
