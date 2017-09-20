#ifndef DANAOS_MAIN_KERNEL_H_
#define DANAOS_MAIN_KERNEL_H_

#include "devices/cga.h"
#include "main/console.h"
#include "boot/multiboot2.h"
#include "elf/elf.h"


struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
};

class kernel {
private:
public:
    kernel(const multiboot_info *mbinfo);
    
    static devices::cga m_cga;
    static console m_console;
    static elf::symbol_lookup m_elf_lookup;
};

#endif // DANAOS_MAIN_KERNEL_H_
