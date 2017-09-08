#include "elf.h"
#include "libk/string.h"
#include "main/kernel.h"

namespace elf {

    elf_symbol_lookup::elf_symbol_lookup(const multiboot_tag_elf_sections *header) {
        auto sectionHeader = reinterpret_cast<const section_header *>(header->sections);
    
        const char *symbolHeaderStringTable =
            reinterpret_cast<const char *>(sectionHeader[header->shndx].addr);
    
        for (unsigned int i = 0; i < header->num; i++) {
            const char *name = &symbolHeaderStringTable[sectionHeader[i].name];
            if (!std::strncmp(name, ".strtab", 7)) {
                stringTable = reinterpret_cast<const char *>(sectionHeader[i].addr);
                strings     = sectionHeader[i].size;
            } else if (!std::strncmp(name, ".symtab", 7)) {
                symbolTable = reinterpret_cast<symbol *>(sectionHeader[i].addr);
                symbols     = sectionHeader[i].size / sizeof(symbol);
            }
        }
    }

    const char *elf_symbol_lookup::lookup(uintptr_t address) {
        const symbol *curr = symbolTable;
        for (size_t i = 0; i < symbols; i++) {
            if ((curr->info & 0x0F) == SYSTEM_TABLE) {
                if ((address >= curr->value) && (address < (curr->value + curr->size))) {
                    return &stringTable[curr->name];
                }
            }
            ++curr;
        }
    
        return "";
    }
    
    void elf_symbol_lookup::print_all() {
        for (size_t i = 0; i < symbols; i++) {
            if ((symbolTable[i].info & 0x0F) == 0x2) {
                kernel::m_console.print("{} ", &stringTable[symbolTable[i].name]);
            }
        }
    }

} // namespace elf