#include <stdint.h>
#include "elf.h"
#include "multiboot/multiboot2.h"
extern "C" {
	#include "devices/cga.h"
}

extern void hang() __asm__("_hang") __attribute__((noreturn));

extern "C" void boot_main(elf::header *elf, multiboot_tag_memmap *mmap) {
	// TODO: try to find the multiboot magic thingy

	uintptr_t entry_addr = load_elf(elf);
	if(entry_addr == 0) {
		print("Fatal: could not parse a proper entry address!");
		hang();
	}
	
	// Create the multiboot structure
	uintptr_t m_curr = 0x400000;
	auto m_info = reinterpret_cast<multiboot_info *>(m_curr);
	m_curr += sizeof(multiboot_info);
	m_info->size = sizeof(multiboot_info);
	
	// Memory map
	if(mmap != nullptr) {
		auto m_mmap = reinterpret_cast<multiboot_tag_memmap *>(m_curr);
		
		*m_mmap = *mmap;
		for(unsigned int i = 0; i < (mmap->size - sizeof(multiboot_tag)) / mmap->entry_size; ++i) {
			m_mmap->entries[i] = mmap->entries[i];
		}
		m_mmap->type = MULTIBOOT_TAG_TYPE_MMAP;
		
		m_info->size += m_mmap->size;
		m_curr += m_mmap->size;
	}
	// Elf sections
	{
		auto m_elf = reinterpret_cast<multiboot_tag_elf_sections *>(m_curr);
		m_curr += sizeof(multiboot_tag_elf_sections);
		
		m_elf->type = MULTIBOOT_TAG_TYPE_ELF_SECTIONS;
		m_elf->num = elf->e_shnum;
		m_elf->entsize = elf->e_shentsize;
		m_elf->shndx = elf->e_shstrndx;
		m_elf->size = sizeof(multiboot_tag_elf_sections) + sizeof(elf::section_header) * elf->e_shnum;

		auto m_section_headers = reinterpret_cast<elf::section_header*>(m_elf->sections);
		auto elf_section_headers = reinterpret_cast<elf::section_header*>(reinterpret_cast<uintptr_t>(elf) + elf->e_shoff);
		for(unsigned int i = 0; i < elf->e_shnum; ++i) {
			m_section_headers[i] = elf_section_headers[i];
		}
		
		m_info->size += m_elf->size;
		m_curr += m_elf->size;
	}
	
	// Set multiboot stuff and call the kernel from asm to keep the registers
	__asm__ volatile(
		"movl	$0x36d76289, %%eax\t\n"
		"movl	%0, %%ebx\t\n"
		"jmp	*%1\t\n"
		:
		: "m"(m_info), "d"(entry_addr)
		: "%eax", "%ebx"
	);
	
}
