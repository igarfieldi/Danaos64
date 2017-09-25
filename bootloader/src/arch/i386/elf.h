#ifndef DANAOS_BOOTLOADER_ARCH_I386_ELF_H_
#define DANAOS_BOOTLOADER_ARCH_I386_ELF_H_

#include <stdint.h>

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
			CURRENT			= 1
		};
		
		struct identification {
			unsigned char ei_magic[4];
			file_class ei_class;
			endian ei_data;
			version ei_version;
			unsigned char ei_pad[6];
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
    
    struct program_header {
    	enum class type : Elf32_Word {
    		PH_NULL			= 0,
    		LOAD			= 1,
    		DYNAMIC			= 2,
    		INTERP			= 3,
    		NOTE			= 4,
    		SHLIB			= 5,
    		PHDR			= 6,
    		TLS				= 7,
    		LOOS			= 0x60000000,
    		HIOS			= 0x6FFFFFFF,
    		LOPROC			= 0x70000000,
    		HIPROC			= 0x7FFFFFFF
    	};
		
		type p_type;
		Elf32_Off p_offset;
		Elf32_Addr p_vaddr;
		Elf32_Addr p_paddr;
		Elf32_Word p_filesz;
		Elf32_Word p_memsz;
		struct permissions {
			Elf32_Word execute			: 1;
			Elf32_Word write			: 1;
			Elf32_Word read				: 1;
			Elf32_Word unspecified		: 5;
			Elf32_Word padding			: 24;
		} __attribute__((packed)) p_flags;
		Elf32_Word p_align;
    } __attribute__((packed));
} // namespace elf


extern "C" uintptr_t parse_elf(uintptr_t header_addr);

#endif //DANAOS_BOOTLOADER_ARCH_I386_ELF_H_
