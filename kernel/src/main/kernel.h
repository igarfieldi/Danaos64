#ifndef DANAOS_MAIN_KERNEL_H_
#define DANAOS_MAIN_KERNEL_H_

#include "devices/cga.h"
#include "main/console.h"
#include "boot/multiboot2.h"


struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
};

class kernel {
private:
    kernel(const multiboot_info *mbinfo);

public:
    static devices::cga m_cga;
    static console m_console;
};

#endif // DANAOS_MAIN_KERNEL_H_