#include "elf.h"
extern "C" {
	#include "devices/cga.h"
}

extern "C" uintptr_t parse_elf(uintptr_t header_addr) {
	auto header = reinterpret_cast<elf::header *>(header_addr);
	
	cga_clear();
	
	// Check if it's an ELF file at all
	if((header->e_ident.ei_magic[0] != 0x7F) || (header->e_ident.ei_magic[1] != 'E') ||
		(header->e_ident.ei_magic[2] != 'L') || (header->e_ident.ei_magic[3] != 'F')) {
		print("Fatal: not an ELF file!");
		return 0;
	}
	// Check the intended architecture
	if(header->e_ident.ei_class != elf::header::file_class::ELF_32) {
		print("Fatal: not an ELF32 file!");
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
	if(header->e_machine != elf::header::machine::I386) {
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
		}
	}
	
	// Return the entry point to jump to
	return header->e_entry;
}
