#ifndef DANAOS_BOOTLOADER_ARCH_I386_ELF_H_
#define DANAOS_BOOTLOADER_ARCH_I386_ELF_H_

#include <stdint.h>

namespace elf {
	using Elf64_Addr = uint64_t;
	using Elf64_Off = uint64_t;
	using Elf64_Half = uint16_t;
	using Elf64_Word = uint32_t;
	using Elf64_Sword = int32_t;
	using Elf64_Xword = uint64_t;
	using Elf64_Sxword = int64_t;
	
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
	
		enum class type : Elf64_Half {
			NONE			= 0,
			REL				= 1,
			EXEC			= 2,
			DYN				= 3,
			CORE			= 5,
			LOPROC			= 0xFF00,
			HIPROC			= 0xFFFF
		};
		
		enum class machine : Elf64_Half {
			NONE			= 0,
			M32				= 1,
			SPARC			= 2,
			I386			= 3,
			M68K			= 4,
			M88K			= 5,
			M860			= 7,
			MIPS			= 8,
			POWERPC			= 20,
			ARM				= 40,
			IA_64			= 50,
			X86_64			= 62,
			AARCH64			= 183,
			RISC_V			= 243
		};
		
		enum class version : Elf64_Word {
			NONE			= 0,
			CURRENT			= 1
		};
		
		enum class abi : unsigned char {
			SYSV			= 0,
			HPUX			= 1,
			STANDALONE		= 255
		};
		
		struct identification {
			unsigned char ei_magic[4];
			file_class ei_class;
			endian ei_data;
			version ei_version;
			abi ei_osiabi;
			unsigned char ei_abiversion;
			unsigned char ei_pad[4];
		} __attribute__((packed)) e_ident;
		type e_type;
		machine e_machine;
		version e_version;
		Elf64_Addr e_entry;
		Elf64_Off e_phoff;
		Elf64_Off e_shoff;
		Elf64_Word e_flags;
		Elf64_Half e_ehsize;
		Elf64_Half e_phentsize;
		Elf64_Half e_phnum;
		Elf64_Half e_shentsize;
		Elf64_Half e_shnum;
		Elf64_Half e_shstrndx;
	} __attribute__((packed));
	
	struct section_header {
		enum class type : Elf64_Word {
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
		
		enum class flags : Elf64_Word {
			WRITE			= 0x1,
			ALLOC			= 0x2,
			EXECINSTR		= 0x4,
			MASKPROC		= 0xF0000000
		};
		
        Elf64_Word sh_name;
        type sh_type;
        Elf64_Xword sh_flags;
        Elf64_Addr sh_addr;
        Elf64_Off sh_offset;
        Elf64_Xword sh_size;
        Elf64_Word sh_link;
        Elf64_Word sh_info;
        Elf64_Xword sh_addralign;
        Elf64_Xword sh_entsize;
    } __attribute__((packed));
    
	enum class symbol_binding : unsigned char {
		LOCAL			= 0,
		GLOBAL			= 1,
		WEAK			= 2,
		LOOS			= 10,
		HIOS			= 12,
		LOPROC			= 13,
		HIPROC			= 15
	};
	
	enum class symbol_type : unsigned char {
		NOTYPE			= 0,
		OBJECT			= 1,
		FUNC			= 2,
		SECTION			= 3,
		FILE			= 4,
		LOOS			= 10,
		HIOS			= 12,
		LOPROC			= 13,
		HIPROC			= 15
	};
	
	struct symbol {
        Elf64_Word st_name;
        struct info_t {
        	symbol_type type		: 4;
        	symbol_binding binding	: 4;
        } __attribute__((packed)) st_info;
        unsigned char st_other;
        Elf64_Half st_shndx;
        Elf64_Addr st_value;
        Elf64_Xword st_size;
    } __attribute__((packed));
    
    struct program_header {
    	enum class type : Elf64_Word {
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
		struct permissions {
			Elf64_Word execute			: 1;
			Elf64_Word write			: 1;
			Elf64_Word read				: 1;
			Elf64_Word unspecified		: 5;
			Elf64_Word padding			: 24;
		} __attribute__((packed)) p_flags;
		Elf64_Off p_offset;
		Elf64_Addr p_vaddr;
		Elf64_Addr p_paddr;
		Elf64_Xword p_filesz;
		Elf64_Xword p_memsz;
		Elf64_Xword p_align;
    } __attribute__((packed));
} // namespace elf


extern "C" uintptr_t parse_elf(uintptr_t header_addr);

#endif //DANAOS_BOOTLOADER_ARCH_I386_ELF_H_
