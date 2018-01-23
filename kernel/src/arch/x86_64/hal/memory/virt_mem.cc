#include "virt_mem.h"
#include "hal/memory/phys_mem.h"
#include "main/kernel.h"

// Comes from the linker
extern uintptr_t KERNEL_VIRT_OFFSET;
extern uintptr_t KERNEL_PHYS_BEGIN;
extern uintptr_t KERNEL_VIRT_BEGIN;
extern uintptr_t KERNEL_VIRT_END;
extern hal::page_map _pml4;

namespace hal {

    void virt_mem_manager::set_page_directory(uintptr_t phys_dir_addr) {
        __asm__ volatile(
            "movl   %0, %%cr3"
            : 
            : "r"(phys_dir_addr)
            :
        );
    }

    virt_mem_manager::virt_mem_manager() noexcept : m_page_map(&_pml4) {
        /* TODO: recursive mapping */
        uintptr_t virtual_offset = reinterpret_cast<uintptr_t>(&KERNEL_VIRT_OFFSET);
        uintptr_t page_map_phys = reinterpret_cast<uintptr_t>(m_page_map) - virtual_offset;

        // Make the page tables accessible
        //m_page_directory[page_table::ENTRIES - 1] = page_dir_entry(page_dir_phys, true, true, false, true, true);
    }

    virt_mem_manager &virt_mem_manager::instance() noexcept {
        static virt_mem_manager inst;
        return inst;
    }

    void virt_mem_manager::init() noexcept {
        while(true);
    }

    uintptr_t virt_mem_manager::map(uintptr_t virt, uintptr_t phys, bool kernel,
                        bool read_only, bool global) noexcept {
        (void)virt;
        (void)phys;
        (void)kernel;
        (void)read_only;
        (void)global;

        size_t table = table_index(virt);
        size_t dir = dir_index(virt);
        size_t dir_ptr = dir_ptr_index(virt);
        size_t map = map_index(virt);

        uintptr_t entry_mappings[3];
        size_t needed_mappings = 0;

        // Check the top-level map
        if(!m_page_map[map].is_present()) {
            // Reserve RAM for the page directory pointer and create the mapping
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
            m_page_map[map] = page_map_entry(addr, true, true, !kernel, false, false, false);
            entry_mappings[needed_mappings++] = addr;
        } else if(!m_page_map[map].is_user()) {
            // Do something about this... users shouldn't map into kernel space
        }

        volatile page_dir_ptr *pdp = reinterpret_cast<volatile page_dir_ptr *>(m_page_map[map].get_dir_ptr_addr());
        // Check the page-directory-pointer
        if(!pdp[dir_ptr].is_present()) {
            // Reserve RAM for the page directory and create the mapping
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
            pdp[dir_ptr] = page_dir_ptr_entry(addr, true, true, !kernel, false, false, false);
            entry_mappings[needed_mappings++] = addr;
        }

        volatile page_dir *pd = reinterpret_cast<volatile page_dir *>(pdp[dir_ptr].get_dir_addr());
        // Check the page-directory
        if(!pd[dir].is_present()) {
            // Reserve RAM for the page table and create the mapping
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
            pd[dir] = page_dir_entry(addr, true, true, !kernel, false, false, false);
            entry_mappings[needed_mappings++] = addr;
        }

        volatile page_table *pt = reinterpret_cast<volatile page_table *>(pd[dir].get_table_addr());
        // Add the table mapping
        pt[table] = page_table_entry(phys, true, !read_only, !kernel, false, false, false);

        // Recursively add the (possibly) needed mappings for paging structures
        for(size_t i = 0; i < needed_mappings; ++i) {
            // TODO: we need the recursive mapping for this
        }
        
        return virt;
    }

    uintptr_t virt_mem_manager::map(uintptr_t virt, uintptr_t phys, size_t count, bool kernel,
                        bool read_only, bool global) noexcept {
        (void)virt;
        (void)phys;
        (void)count;
        (void)kernel;
        (void)read_only;
        (void)global;
        return 0;
    }
    uintptr_t virt_mem_manager::map_range(uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys,
                        bool kernel, bool read_only, bool global) noexcept {
        (void)virt_start;
        (void)virt_end;
        (void)phys;
        (void)kernel;
        (void)read_only;
        (void)global;
        return 0;
    }

    /*bool virt_mem_manager::test_and_map(uintptr_t virt, uintptr_t phys, bool kernel,
                        bool read_only, bool global) noexcept {
        
    }

    bool virt_mem_manager::test_and_map(uintptr_t virt, uintptr_t phys, size_t count, bool kernel,
                        bool read_only, bool global) noexcept {
        
    }
    bool virt_mem_manager::test_and_map_range(uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys,
                        bool kernel, bool read_only, bool global) noexcept {
        
    }*/

} // namespace hal
