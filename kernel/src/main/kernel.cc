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
#include "devices/cga.h"
#include "libk/math.h"
#include "debug/trace.h"
#include "hal/memory/memmanager.h"

console kernel::m_console;
elf::symbol_lookup kernel::m_elf_lookup;

kernel::kernel(const multiboot_info *mbinfo) {
	(void)mbinfo;
	debug::backtrace(2);
}

extern "C" void kernelMain(uint32_t magic, uintptr_t info) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        kernel::m_console.print("Error: The kernel was not loaded by a multiboot loader!");
        while(true);
    }

    
    kernel::m_console.print("Loaded!");
    while(true);

    multiboot_info *mbinfo = reinterpret_cast<multiboot_info*>(info);
    multiboot_tag_elf_sections *elf_sections = nullptr;
    multiboot_tag_framebuffer_common *framebuffer = nullptr;
    multiboot_tag_mmap* memmap = nullptr;

    unsigned int byte = 8;
    while(byte < mbinfo->size) {
        uint32_t type = reinterpret_cast<multiboot_tag*>(info + byte)->type;
        uint32_t size = reinterpret_cast<multiboot_tag*>(info + byte)->size;
        switch(type) {
            /*case MULTIBOOT_TAG_TYPE_CMDLINE:
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
                kernel::m_console.print("Boot device: []\n", reinterpret_cast<multiboot_tag_bootdev*>(info + byte)->biosdev);
                break;*/
            case MULTIBOOT_TAG_TYPE_MMAP: {
                memmap = reinterpret_cast<multiboot_tag_mmap*>(info + byte);
                break;
            }
            /*case MULTIBOOT_TAG_TYPE_VBE:
                kernel::m_console.print("VBE mode: {}\n", reinterpret_cast<multiboot_tag_vbe*>(info + byte)->vbe_mode);
                break;*/
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
                framebuffer = reinterpret_cast<multiboot_tag_framebuffer_common*>(info + byte);
                break;
            }
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: {
                elf_sections = reinterpret_cast<multiboot_tag_elf_sections*>(info + byte);
                break;
            }
            /*case MULTIBOOT_TAG_TYPE_APM: {
                auto apm = reinterpret_cast<multiboot_tag_apm*>(info + byte);
                kernel::m_console.print("APM version: {}\n", apm->version);
                break;
            }
            case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
                auto acpi = reinterpret_cast<multiboot_tag_old_acpi*>(info + byte);
                kernel::m_console.print("ACPI root: {}\n", reinterpret_cast<char*>(acpi->rsdp));
                break;
            }*/
        }

        // Align the tag structure
        byte += size;
        if(byte % 8 != 0) {
            byte += 8 - (byte % 8);
        }
    }
    
    if(memmap != nullptr) {
        hal::memory_manager::instance().init(hal::multiboot_memmap(*memmap));
    }
    if(framebuffer != nullptr) {
    	devices::cga::instance().init(framebuffer->framebuffer_addr, framebuffer->framebuffer_width,
                        framebuffer->framebuffer_height);
        //devices::cga::instance().clear();
        kernel::m_console.print("Framebuffer initialized at [] ({}/{})\n", devices::cga::instance().buffer_address(),
                            devices::cga::instance().width(), devices::cga::instance().height());
    } else {
    	devices::cga::instance().init(0xB8000, 80, 25);
    }
    if(elf_sections != nullptr) {
        // TODO: make those addresses virtual!
		kernel::m_elf_lookup.init(elf_sections);
		debug::backtrace(2);
    }

    kernel::m_console.print("Kernel loaded!\n");


    while(true);
}
