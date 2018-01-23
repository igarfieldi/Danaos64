#include "elf.h"
#include "libk/string.h"
#include "main/kernel.h"
#include "hal/memory/memmanager.h"

namespace elf {

    const section_header *find_string_table(const char *strHeaderTbl, const section_header *headers, size_t count) {
    	for (unsigned int i = 0; i < count; i++) {
            const char *name = &strHeaderTbl[headers[i].sh_name];
            if (!std::strncmp(name, ".strtab", 7)) {
                return &headers[i];
            }
        }
        return nullptr;
    }
    
    const section_header *find_symbol_table(const char *strHeaderTbl, const section_header *headers, size_t count) {
    	for (unsigned int i = 0; i < count; i++) {
            const char *name = &strHeaderTbl[headers[i].sh_name];
            if (!std::strncmp(name, ".symtab", 7)) {
                return &headers[i];
            }
        }
        return nullptr;
    }
    
	symbol_lookup::symbol_lookup() : symbolTable(nullptr), stringTable(nullptr),
									symbols(0), strings(0) {
	}

    void symbol_lookup::init(const char *stringTable, size_t strings, const symbol *symbolTable, size_t symbols) {
        // Alloc page frames and map the string and the symbol table into virtual memory
        if(stringTable != nullptr) {
        	this->strings = strings;
		    this->stringTable = stringTable;
		}
        if(symbolTable != nullptr) {
        	this->symbols = symbols;
		    this->symbolTable = symbolTable;
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
