#include "virt_mem.h"
#include "hal/memory/phys_mem.h"
#include "main/kernel.h"

// Comes from the linker
extern uintptr_t KERNEL_VIRT_OFFSET;
extern uintptr_t KERNEL_PHYS_BEGIN;
extern uintptr_t KERNEL_VIRT_BEGIN;
extern uintptr_t KERNEL_VIRT_END;
extern hal::page_directory_entry _page_directory;

namespace hal {

    void virt_mem_manager::set_page_directory(uintptr_t phys_dir_addr) {
        __asm__ volatile(
            "movl   %0, %%cr3"
            : 
            : "r"(phys_dir_addr)
            :
        );
    }

    virt_mem_manager::virt_mem_manager() noexcept : m_page_directory(&_page_directory),
            m_tables(reinterpret_cast<page_table *>(PAGE_SIZE * page_table::ENTRIES * (page_table::ENTRIES - 1))) {
        // The last page directory shall point to our page tables so we can modify them by accessing that

        uintptr_t virtual_offset = reinterpret_cast<uintptr_t>(&KERNEL_VIRT_OFFSET);
        uintptr_t page_dir_phys = reinterpret_cast<uintptr_t>(m_page_directory) - virtual_offset;
        // Make the page tables accessible
        m_page_directory[page_table::ENTRIES - 1] = page_directory_entry(true, true, false, true, true, false, page_dir_phys);
    }

    virt_mem_manager &virt_mem_manager::instance() noexcept {
        static virt_mem_manager inst;
        return inst;
    }

    void virt_mem_manager::init() noexcept {
        uintptr_t virtual_offset = reinterpret_cast<uintptr_t>(&KERNEL_VIRT_OFFSET);
        uintptr_t page_dir_phys = reinterpret_cast<uintptr_t>(m_page_directory) - virtual_offset;

        // Map page directory
        this->map(reinterpret_cast<uintptr_t>(m_page_directory), page_dir_phys, true, false, false);
        // Map the kernel pages
        // TODO: distinction into read-only and such
        this->map_range(reinterpret_cast<uintptr_t>(&KERNEL_VIRT_BEGIN),
                    reinterpret_cast<uintptr_t>(&KERNEL_VIRT_END),
                    reinterpret_cast<uintptr_t>(&KERNEL_PHYS_BEGIN),
                    true, false, false);

        set_page_directory(page_dir_phys);
    }

    uintptr_t virt_mem_manager::map(uintptr_t virt, uintptr_t phys, bool kernel,
                        bool read_only, bool global) noexcept {
        size_t dir = dir_index(virt);
        size_t table = table_index(virt);

        if(!m_page_directory[dir].present) {
            m_page_directory[dir] = page_directory_entry(true, true, true,
                true, false, false, phys_mem_manager::instance().alloc_any());
            m_tables[dir] = page_table();
        }

        m_tables[dir][table] = page(true, !read_only, !kernel, true, false, global, phys);
        return virt;
    }

    uintptr_t virt_mem_manager::map(uintptr_t virt, uintptr_t phys, size_t count, bool kernel,
                        bool read_only, bool global) noexcept {
        for(size_t curr_page = 0; curr_page < count; ++curr_page) {
            size_t dir = dir_index(virt + curr_page*PAGE_SIZE);
            size_t table = table_index(virt + curr_page*PAGE_SIZE);

            if(!m_page_directory[dir].present) {
                m_page_directory[dir] = page_directory_entry(true, true, true,
                    true, false, false, phys_mem_manager::instance().alloc_any());
                m_tables[dir] = page_table();
            }

            m_tables[dir][table] = page(true, !read_only, !kernel, true, false, global, phys + curr_page*PAGE_SIZE);
        }

        return virt;
    }
    uintptr_t virt_mem_manager::map_range(uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys,
                        bool kernel, bool read_only, bool global) noexcept {
        uintptr_t curr_virt = virt_start;
        while(curr_virt < virt_end) {
            size_t dir = dir_index(curr_virt);
            size_t table = table_index(curr_virt);

            if(!m_page_directory[dir].present) {
                m_page_directory[dir] = page_directory_entry(true, true, true,
                    true, false, false, phys_mem_manager::instance().alloc_any());
                m_tables[dir] = page_table();
            }

            m_tables[dir][table] = page(true, !read_only, !kernel, true, false, global, phys);
            curr_virt += PAGE_SIZE;
            phys += PAGE_SIZE;
        }
        
        return virt_start;
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