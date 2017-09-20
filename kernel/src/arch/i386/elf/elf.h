#ifndef DANAOS_ELF_ELF_H_
#define DANAOS_ELF_ELF_H_

#include <stdint.h>
#include "boot/multiboot2.h"

namespace elf {
	using Elf32_Addr = uint32_t;
	using Elf32_Half = uint16_t;
	using Elf32_Off = uint32_t;
	using Elf32_Sword = int32_t;
	using Elf32_Word = uint32_t;
	
	struct header {
		enum class file_class : unsigned char {
			NONE			= 0,
			ELF_32			= 1,
			ELF_64			= 2
		};
		
		enum class endian : unsigned char {
			NONE			= 0,
			LSB				= 1,
			MSB				= 2
		};
	
		enum class type : Elf32_Half {
			NONE			= 0,
			REL				= 1,
			EXEC			= 2,
			DYN				= 3,
			CORE			= 5,
			LOPROC			= 0xFF00,
			HIPROC			= 0xFFFF
		};
		
		enum class machine : Elf32_Half {
			NONE			= 0,
			M32				= 1,
			SPARC			= 2,
			I386			= 3,
			M68K			= 4,
			M88K			= 5,
			M860			= 7,
			MIPS			= 8
		};
		
		enum class version : Elf32_Word {
			NONE			= 0,
			CURRENT		= 1
		};
		
		struct identification {
			unsigned char ei_magic[4];
			file_class ei_class;
			endian ei_data;
			version ei_version;
			unsigned char ei_pad[9];
		} __attribute__((packed)) e_ident;
		type e_type;
		machine e_machine;
		version e_version;
		Elf32_Addr e_entry;
		Elf32_Off e_phoff;
		Elf32_Off e_shoff;
		Elf32_Word e_flags;
		Elf32_Half e_ehsize;
		Elf32_Half e_phentsize;
		Elf32_Half e_phnum;
		Elf32_Half e_shentsize;
		Elf32_Half e_shnum;
		Elf32_Half e_shstrndx;
	} __attribute__((packed));
	
	struct section_header {
		enum class type : Elf32_Word {
			SHTNULL			= 0,
			PROGBITS		= 1,
			SYMTAB			= 2,
			STRTAB			= 3,
			RELA			= 4,
			HASH			= 5,
			DYNAMIC			= 6,
			NOTE			= 7,
			NOBITS			= 8,
			REL				= 9,
			SHLIB			= 10,
			DYNSYM			= 11,
			LOPROC			= 0x70000000,
			HIPROC			= 0x7FFFFFFF,
			LOUSER			= 0x80000000,
			HIUSER			= 0xFFFFFFFF
		};
		
		enum class flags : Elf32_Word {
			WRITE			= 0x1,
			ALLOC			= 0x2,
			EXECINSTR		= 0x4,
			MASKPROC		= 0xF0000000
		};
		
        Elf32_Word sh_name;
        type sh_type;
        Elf32_Word sh_flags;
        Elf32_Addr sh_addr;
        Elf32_Off sh_offset;
        Elf32_Word sh_size;
        Elf32_Word sh_link;
        Elf32_Word sh_info;
        Elf32_Word sh_addralign;
        Elf32_Word sh_entsize;
    } __attribute__((packed));
    
	enum class symbol_binding : unsigned char {
		LOCAL			= 0,
		GLOBAL			= 1,
		WEAK			= 2,
		LOPROC			= 13,
		HIPROC			= 15
	};
	
	enum class symbol_type : unsigned char {
		NOTYPE			= 0,
		OBJECT			= 1,
		FUNC			= 2,
		SECTION			= 3,
		FILE			= 4,
		LOPROC			= 13,
		HIPROC			= 15
	};
	
	struct symbol {
        Elf32_Word st_name;
        Elf32_Addr st_value;
        Elf32_Word st_size;
        struct info_t {
        	symbol_type type		: 4;
        	symbol_binding binding	: 4;
        } __attribute__((packed)) st_info;
        unsigned char st_other;
        Elf32_Half st_shndx;
    } __attribute__((packed));

    class elf_symbol_lookup {
    private:
        const symbol *symbolTable;
        const char *stringTable;
        Elf32_Word symbols;
        Elf32_Word strings;
    
    public:
        elf_symbol_lookup(const multiboot_tag_elf_sections *headers);
    
        const char *lookup(uintptr_t address);
        void print_all();
    };
} // namespace elf

#endif //DANAOS_ELF_ELF_H_
