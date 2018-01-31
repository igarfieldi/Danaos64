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
#include "hal/memory/memmanager.h"
#include "hal/hal.h"
#include "hal/interrupts/interrupts.h"
#include "main/task/scheduler.h"

console kernel::m_console;
elf::symbol_lookup kernel::m_elf_lookup;

kernel::kernel(const multiboot_info *mbinfo) {
	(void)mbinfo;
	debug::backtrace(2);
}

static void test1() {
    for(size_t i = 0; i < 5000; ++i) {
        kernel::m_console.print("A: {}\n", i);
        volatile unsigned long long j = 0;
        for(unsigned long long v = 1; v < 3000000; ++v) {
            j += math::logull(10, v);
        }
        task::scheduler::instance().yield();
        //task::scheduler::instance().schedule();
    }
}

static void test2() {
    for(size_t i = 0; i < 4000; ++i) {
        kernel::m_console.print("B: {}\n", i);
        volatile unsigned long long j = 0;
        for(unsigned long long v = 1; v < 3000000; ++v) {
            j += math::logull(10, v);
        }
        task::scheduler::instance().yield();
        //task::scheduler::instance().schedule();
    }
}

static void test3() {
    for(size_t i = 0; i < 3000; ++i) {
        kernel::m_console.print("C: {}\n", i);
        volatile unsigned long long j = 0;
        for(unsigned long long v = 1; v < 3000000; ++v) {
            j += math::logull(10, v);
        }
        task::scheduler::instance().yield();
        //task::scheduler::instance().schedule();
    }
    debug::backtrace();
}

static void run_kernel() {
    task::task task2(test2);
    task::task task3(test3);
    task::scheduler::instance().ready(task2);
    task::scheduler::instance().ready(task3);

    test1();
    while(true) {
        task::scheduler::instance().yield();
    }
}

extern "C" void kernelMain(uint32_t magic, uintptr_t info) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        kernel::m_console.print("Error: The kernel was not loaded by a multiboot loader!");
        while(true);
    }

    multiboot_info *mbinfo = reinterpret_cast<multiboot_info *>(info);
    const multiboot_tag_elf_sections *elf_sections = nullptr;
    const multiboot_tag_framebuffer_common *framebuffer = nullptr;
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

    // Save the framebuffer data to access it after virtual address transition
    uintptr_t fb_addr = 0xB8000;
    uintptr_t fb_width = 80;
    uintptr_t fb_height = 25;
    // Save elf section size for mapping
    size_t elf_section_size = 0;
    const elf::section_header *sectionHeaders = nullptr;
    const elf::section_header *stringHeader = nullptr;
    const elf::section_header *symbolHeader = nullptr;
    const char *strHeaderTbl = nullptr;
    
    // Initialize physical memory management
    if(memmap != nullptr) {
        hal::phys_mem_manager::instance().init(hal::multiboot_memmap(*memmap));
    }

	// Save framebuffer data
    if(framebuffer != nullptr) {
        fb_addr = framebuffer->framebuffer_addr;
        fb_width = framebuffer->framebuffer_width;
        fb_height = framebuffer->framebuffer_height;
    }
    if(elf_sections != nullptr) {
    	elf_section_size = elf_sections->size;
    	// Walk through the sections and mark their memory areas as used
    	hal::phys_mem_manager::instance().alloc_range(reinterpret_cast<uintptr_t>(elf_sections), elf_section_size);
        sectionHeaders = reinterpret_cast<const elf::section_header *>(elf_sections->sections);
    
        strHeaderTbl = reinterpret_cast<const char *>(sectionHeaders[elf_sections->shndx].sh_addr);
    	hal::phys_mem_manager::instance().alloc_range(sectionHeaders[elf_sections->shndx].sh_addr,
    		sectionHeaders[elf_sections->shndx].sh_size);
     	
     	// We need to find the symbol and string tables
     	stringHeader = find_string_table(strHeaderTbl, sectionHeaders, elf_sections->num);
     	symbolHeader = find_symbol_table(strHeaderTbl, sectionHeaders, elf_sections->num);
        if(stringHeader != nullptr) {
			hal::phys_mem_manager::instance().alloc_range(stringHeader->sh_addr, stringHeader->sh_size);
		}
        if(symbolHeader != nullptr) {
			hal::phys_mem_manager::instance().alloc_range(symbolHeader->sh_addr, symbolHeader->sh_size);
		}
    }
    
    hal::virt_mem_manager::instance().init();
    hal::memory_manager::instance().init();

    // Initialize framebuffer	
    devices::cga::instance().init(fb_addr, fb_width, fb_height);

	// Initialize elf lookup and map the tables into VM
    if(elf_sections != nullptr) {
    	elf_sections = reinterpret_cast<multiboot_tag_elf_sections *>(
    		hal::memory_manager::instance().kernel_alloc_pages(
    			reinterpret_cast<uintptr_t>(elf_sections),
		    	elf_section_size));
		
        sectionHeaders = reinterpret_cast<const elf::section_header *>(elf_sections->sections);
        strHeaderTbl = reinterpret_cast<const char *>(
				hal::memory_manager::instance().kernel_alloc_pages(
					sectionHeaders[elf_sections->shndx].sh_addr,
					sectionHeaders[elf_sections->shndx].sh_size));
        
		// Find the headers again in the virtualized headers
		const char *stringTable = nullptr;
		const elf::symbol *symbolTable = nullptr;
		size_t strings = 0;
		size_t symbols = 0;
     	stringHeader = find_string_table(strHeaderTbl, sectionHeaders, elf_sections->num);
     	symbolHeader = find_symbol_table(strHeaderTbl, sectionHeaders, elf_sections->num);
     	
     	// Virtualize the sections
     	if(stringHeader != nullptr) {
			stringTable = reinterpret_cast<const char *>(
				hal::memory_manager::instance().kernel_alloc_pages(
					stringHeader->sh_addr,
					stringHeader->sh_size));
			strings = stringHeader->sh_size / sizeof(char);
		}
        if(symbolHeader != nullptr) {
			symbolTable = reinterpret_cast<const elf::symbol *>(
				hal::memory_manager::instance().kernel_alloc_pages(
					symbolHeader->sh_addr,
					symbolHeader->sh_size));
			symbols = symbolHeader->sh_size / sizeof(elf::symbol);
		}
		kernel::m_elf_lookup.init(stringTable, strings, symbolTable, symbols);
    }

    hal::init_hal();

    kernel::m_console.print("Kernel loaded!\n");


    hal::test_isr test;
    hal::isr_dispatcher::instance().register_isr(0, test);

    task::scheduler::instance().start(&run_kernel);
    //test_print();

    while(true);
}
