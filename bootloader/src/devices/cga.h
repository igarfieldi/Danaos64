#ifndef DANAOS_BOOTLOADER_DEVICES_CGA_H_
#define DANAOS_BOOTLOADER_DEVICES_CGA_H_

const unsigned int CGA_WIDTH = 80;
const unsigned int CGA_HEIGHT = 25;
const unsigned int CGA_ATTRIBUTE = 15;

void cga_clear();
void print(const char *msg);

#endif //DANAOS_BOOTLOADER_DEVICES_CGA_H_
