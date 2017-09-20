#include "elf.h"
#include "libk/string.h"
#include "main/kernel.h"

namespace elf {

	symbol_lookup::symbol_lookup() : symbolTable(nullptr), stringTable(nullptr),
									symbols(0), strings(0) {
	}

    void symbol_lookup::init(const multiboot_tag_elf_sections *header) {
        auto sectionHeader = reinterpret_cast<const section_header *>(header->sections);
    
        const char *symbolHeaderStringTable =
            reinterpret_cast<const char *>(sectionHeader[header->shndx].sh_addr);
    
        for (unsigned int i = 0; i < header->num; i++) {
            const char *name = &symbolHeaderStringTable[sectionHeader[i].sh_name];
            if (!std::strncmp(name, ".strtab", 7)) {
                stringTable = reinterpret_cast<const char *>(sectionHeader[i].sh_addr);
                strings     = sectionHeader[i].sh_size;
            } else if (!std::strncmp(name, ".symtab", 7)) {
                symbolTable = reinterpret_cast<symbol *>(sectionHeader[i].sh_addr);
                symbols     = sectionHeader[i].sh_size / sizeof(symbol);
            }
        }
    }

    const char *symbol_lookup::lookup(uintptr_t address) {
        const symbol *curr = symbolTable;
        for (size_t i = 0; i < symbols; i++) {
            if (curr->st_info.type == symbol_type::FUNC) {
                if ((address >= curr->st_value) && (address < (curr->st_value + curr->st_size))) {
                    return &stringTable[curr->st_name];
                }
            }
            ++curr;
        }
    
        return "";
    }
    
    void symbol_lookup::print_all() {
        for (size_t i = 0; i < symbols; i++) {
            if (symbolTable[i].st_info.type == symbol_type::FUNC) {
                kernel::m_console.print("{} ", &stringTable[symbolTable[i].st_name]);
            }
        }
    }

} // namespace elf
