#ifndef DANAOS_ELF_ELF_H_
#define DANAOS_ELF_ELF_H_

#include <stdint.h>
#include "boot/multiboot2.h"

namespace elf {
    class elf_symbol_lookup {
    private:
        struct section_header {
            uint32_t name;
            uint32_t type;
            uint32_t flags;
            uint32_t addr;
            uint32_t offset;
            uint32_t size;
            uint32_t link;
            uint32_t info;
            uint32_t addr_align;
            uint32_t entry_size;
        } __attribute__((packed));
    
        struct symbol {
            uint32_t name;
            uint32_t value;
            uint32_t size;
            uint8_t info;
            uint8_t other;
            uint16_t shndx;
        } __attribute__((packed));
    
        enum Types { SYSTEM_TABLE = 0x2 };
    
        const symbol *symbolTable;
        const char *stringTable;
        uint32_t symbols;
        uint32_t strings;
    
    public:
        elf_symbol_lookup(const multiboot_tag_elf_sections *headers);
    
        const char *lookup(uintptr_t address);
        void print_all();
    };
} // namespace elf

#endif //DANAOS_ELF_ELF_H_