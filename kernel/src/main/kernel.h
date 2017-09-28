#ifndef DANAOS_MAIN_KERNEL_H_
#define DANAOS_MAIN_KERNEL_H_

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

    template < class... Args >
    static void __attribute__((noreturn)) panic(const char *msg, Args... args) {
        kernel::m_console.print("PANIC: ");
        kernel::m_console.print(msg, static_cast<Args&&>(args)...);
        while(true);
    }
    
    static console m_console;
    static elf::symbol_lookup m_elf_lookup;
};

#endif // DANAOS_MAIN_KERNEL_H_
