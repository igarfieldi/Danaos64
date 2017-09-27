#include "elf.h"
extern "C" {
	#include "devices/cga.h"
}

extern "C" uintptr_t load_elf(elf::header *header) {
	uintptr_t header_addr = reinterpret_cast<uintptr_t>(header);
	
	cga_clear();
	
	// Check if it's an ELF file at all
	if((header->e_ident.ei_magic[0] != 0x7F) || (header->e_ident.ei_magic[1] != 'E') ||
		(header->e_ident.ei_magic[2] != 'L') || (header->e_ident.ei_magic[3] != 'F')) {
		print("Fatal: not an ELF file!");
		return 0;
	}
	// Check the intended architecture
	if(header->e_ident.ei_class != elf::header::file_class::ELF_64) {
		print("Fatal: not an ELF64 file!");
		return 0;
	}
	// Check byte order
	if(header->e_ident.ei_data != elf::header::endian::LSB) {
		print("Fatal: unsupported byte order!");
		return 0;
	}
	// Check version
	if(header->e_version != elf::header::version::CURRENT) {
		print("Fatal: unsupported ELF version!");
		return 0;
	}
	// Check machine
	if(header->e_machine != elf::header::machine::X86_64) {
		print("Fatal: unsupported machine!");
		return 0;
	}
	// Check machine
	if(header->e_type != elf::header::type::EXEC) {
		print("Fatal: unsupported ELF type!");
		return 0;
	}
	
	if(header->e_phnum == 0) {
		print("Fatal: couldn't find program header!");
		return 0;
	}
	
	auto program_headers = reinterpret_cast<elf::program_header *>(header_addr + header->e_phoff);
	// Where the ELF currently resides (which is where we will be copying from
	volatile const char *curr_elf = reinterpret_cast<volatile const char *>(header_addr);
	uintptr_t highest_address = 0;
	
	// Load all loadable segments
	for(unsigned int i = 0; i < header->e_phnum; ++i) {
		if(program_headers[i].p_type == elf::program_header::type::LOAD) {
			char *target_mem = reinterpret_cast<char *>(program_headers[i].p_vaddr);
			// What's actually in file (and thus copyable)
			for(unsigned int j = 0; j < program_headers[i].p_filesz; ++j) {
				target_mem[j] = curr_elf[program_headers[i].p_offset + j];
			}
			// What's supposed to be zero-initialized
			for(unsigned int j = program_headers[i].p_filesz; j < program_headers[i].p_memsz; ++j) {
				target_mem[j] = 0;
			}

			uintptr_t upper = program_headers[i].p_vaddr + program_headers[i].p_memsz;
			highest_address = (upper > highest_address) ? upper : highest_address;
		}
	}

	// Load the sections into memory which are not part of the program headers
	auto section_headers = reinterpret_cast<elf::section_header *>(header_addr + header->e_shoff);
	char *buffer = reinterpret_cast<char *>(highest_address);
	for(unsigned int i = 0; i < header->e_shnum; ++i) {
		if(section_headers[i].sh_addr == 0) {
			section_headers[i].sh_addr = reinterpret_cast<elf::Elf64_Addr>(buffer);
			volatile char *section_mem = reinterpret_cast<volatile char *>(header_addr + section_headers[i].sh_offset);
			for(unsigned int j = 0; j < section_headers[i].sh_size; ++j) {
				*(buffer++) = section_mem[j];
			}
		}
	}
	
	// Return the entry point to jump to
	return header->e_entry;
}
