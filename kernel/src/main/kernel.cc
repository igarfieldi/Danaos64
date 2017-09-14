/**
 * main/kernel.cc
 *
 * Contains the entry point for the 'high-level', C++ part of the kernel.
 * Every global object needed is defined here in order of initialization, as
 * well.
**/

#include <stdint.h>
#include "kernel.h"
#include "elf/elf.h"
#include "libk/math.h"

devices::cga kernel::m_cga(80, 25, 0xB8000);
console kernel::m_console;

kernel::kernel(const multiboot_info *mbinfo) {
}

extern "C" void kernelMain(uint32_t magic, uintptr_t info) {
	volatile char *screen = reinterpret_cast<volatile char*>(0xB8000);
	screen[0] = 'A';
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        kernel::m_console.print("Error: The kernel was not loaded by a multiboot loader!");
        while(true);
    }

    kernel::m_console.print("Magic number: []", magic);
    kernel::m_console.print("Hi there!\n");

    multiboot_info *mbinfo = reinterpret_cast<multiboot_info*>(info);

    kernel::m_console.print("Multiboot info size: {}\n", mbinfo->size);
    unsigned int byte = 8;
    unsigned int y = 3;
    while(byte < mbinfo->size) {
        uint32_t type = reinterpret_cast<multiboot_tag*>(info + byte)->type;
        uint32_t size = reinterpret_cast<multiboot_tag*>(info + byte)->size;
        kernel::m_console.print("Segment type {} | size {}\n", type, size);
        ++y;
        switch(type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                kernel::m_console.print("Boot command-line: {}\n", reinterpret_cast<multiboot_tag_string*>(info + byte)->string);
                break;
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                kernel::m_console.print("OEM: {}\n", reinterpret_cast<multiboot_tag_string*>(info + byte)->string);
                break;
            case MULTIBOOT_TAG_TYPE_MODULE:
                kernel::m_console.print("Modules: {}\n", reinterpret_cast<multiboot_tag_module*>(info + byte)->cmdline);
                break;
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                auto meminfo = reinterpret_cast<multiboot_tag_basic_meminfo*>(info + byte);
                kernel::m_console.print("Lower/upper memory: {}/{} KB\n", meminfo->mem_lower, meminfo->mem_upper);
                break;
            }
            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                kernel::m_console.print("Boot device: {}\n", reinterpret_cast<multiboot_tag_bootdev*>(info + byte)->biosdev);
                break;
            case MULTIBOOT_TAG_TYPE_MMAP: {
                auto memmap = reinterpret_cast<multiboot_tag_mmap*>(info + byte);
                kernel::m_console.print("Memory map entries: {}\n", (memmap->size - 16) / memmap->entry_size);
                break;
            }
            case MULTIBOOT_TAG_TYPE_VBE:
                kernel::m_console.print("VBE mode: {}\n", reinterpret_cast<multiboot_tag_vbe*>(info + byte)->vbe_mode);
                break;
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
                auto framebuffer = reinterpret_cast<multiboot_tag_framebuffer_common*>(info + byte);
                kernel::m_console.print("Framebuffer address: []\n", static_cast<uintptr_t>(framebuffer->framebuffer_addr));
                break;
            }
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: {
                auto elf_sections = reinterpret_cast<multiboot_tag_elf_sections*>(info + byte);
                elf::elf_symbol_lookup elf(elf_sections);
                kernel::m_console.print("Kernel entry symbol: {}\n", elf.lookup(reinterpret_cast<uintptr_t>(&kernelMain)));
                break;
            }
            case MULTIBOOT_TAG_TYPE_APM: {
                auto apm = reinterpret_cast<multiboot_tag_apm*>(info + byte);
                kernel::m_console.print("APM version: {}\n", apm->version);
                break;
            }
            case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
                auto acpi = reinterpret_cast<multiboot_tag_old_acpi*>(info + byte);
                kernel::m_console.print("ACPI root: {}\n", reinterpret_cast<char*>(acpi->rsdp));
                break;
            }
        }

        // Align the tag structure
        byte += size;
        if(byte % 8 != 0) {
            byte += 8 - (byte % 8);
        }
    }

    while(true);
}
