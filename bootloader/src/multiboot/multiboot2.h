#ifndef DANAOS_BOOTLOADER_MULTIBOOT_MULTIBOOT2_H_
#define DANAOS_BOOTLOADER_MULTIBOOT_MULTIBOOT2_H_

#define MULTIBOOT2_LOADER_MAGIC					0x36d76289
#define MULTIBOOT_TAG_TYPE_MMAP					6
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER			8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS			9

#include <stdint.h>

using multiboot_uint32_t = uint32_t;
using multiboot_uint64_t = uint64_t;

struct multiboot_info {
	multiboot_uint32_t size;
	multiboot_uint32_t unused;
} __attribute__((packed));

struct multiboot_tag {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
} __attribute__((packed));

struct multiboot_mmap_entry {
	multiboot_uint64_t addr;
	multiboot_uint64_t len;
	#define MULTIBOOT_MEMORY_AVAILABLE		1
	#define MULTIBOOT_MEMORY_RESERVED		2
	multiboot_uint32_t type;
	multiboot_uint32_t zero;
};

struct multiboot_tag_memmap {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint32_t entry_size;
	multiboot_uint32_t entry_version;
	multiboot_mmap_entry entries[0];
} __attribute__((packed));

struct multiboot_tag_elf_sections {
	multiboot_uint32_t type;
	multiboot_uint32_t size;
	multiboot_uint32_t num;
	multiboot_uint32_t entsize;
	multiboot_uint32_t shndx;
	char sections[0];
} __attribute__((packed));

#endif //DANAOS_BOOTLOADER_MULTIBOOT_MULTIBOOT2_H_
