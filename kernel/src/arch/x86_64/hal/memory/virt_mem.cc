#include "virt_mem.h"
#include "hal/memory/phys_mem.h"
#include "main/kernel.h"

// Comes from the linker
extern uintptr_t KERNEL_VIRT_OFFSET;
extern uintptr_t KERNEL_PHYS_BEGIN;
extern uintptr_t KERNEL_VIRT_BEGIN;
extern uintptr_t KERNEL_VIRT_END;
extern hal::page_map_entry *_pml4;
extern hal::page_dir_ptr_entry *_pdp;
extern hal::page_dir_entry *_pd;

namespace hal {

    void virt_mem_manager::set_page_directory(uintptr_t phys_dir_addr) {
        __asm__ (
            "movq   %0, %%cr3"
            : 
            : "r"(phys_dir_addr)
            :
        );
    }

    void virt_mem_manager::map_pre_paging(uintptr_t virt, uintptr_t phys) {
        // TODO: distinction into read-only and such
        // TODO: use large pages if possible?
        size_t pme = map_index(virt);
        size_t pdpe = dir_ptr_index(virt);
        size_t pde = dir_index(virt);
        size_t pte = table_index(virt);

        // Add the mappings to the (physical) structures: pml4, pdp, pd, and pt
        if(!m_page_map_phys->entry(pme).is_present()) {
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
            m_page_map_phys->entry(pme) = page_map_entry(addr, true, true, true, false, false, false);
        }

        // TODO: volatile?
        page_dir_ptr_entry *pdp = reinterpret_cast< page_dir_ptr_entry *>(m_page_map_phys->entry(pme).get_dir_ptr_addr());
        if(!pdp[pdpe].is_present()) {
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
            pdp[pdpe] = page_dir_ptr_entry(addr, true, true, true, false, false, false);
        }

        // TODO: volatile?
        page_dir_entry *pd = reinterpret_cast< page_dir_entry *>(pdp[pdpe].get_dir_addr());
        if(!pd[pde].is_present()) {
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
            pd[pde] = page_dir_entry(addr, true, true, true, false, false, false);
        }

        // TODO: volatile?
        page_table_entry *pt = reinterpret_cast< page_table_entry *>(pd[pde].get_table_addr());
        pt[pte] = page_table_entry(phys, true, true, true, false, false, false, false, false);
    }

    virt_mem_manager::virt_mem_manager() noexcept :
            VIRT_OFFSET(reinterpret_cast<uintptr_t>(&KERNEL_VIRT_OFFSET)),
            m_page_map_phys(nullptr),
            m_page_map(reinterpret_cast< page_map *>(PAGE_MAP_REC)),
            m_page_dir_ptr(reinterpret_cast< page_dir_ptr *>(PAGE_DIR_PTR_REC)),
            m_page_dir(reinterpret_cast< page_dir *>(PAGE_DIR_REC)),
            m_page_table(reinterpret_cast< page_table *>(PAGE_TBL_REC)),
            m_vkernel_start(0), m_vkernel_end(0) {
    }

    virt_mem_manager &virt_mem_manager::instance() noexcept {
        static virt_mem_manager inst;
        return inst;
    }

    void virt_mem_manager::init() noexcept {
        // Allocate a new page frame for the page directory
        uintptr_t page_map_phys = phys_mem_manager::instance().alloc_any();
        m_page_map_phys = reinterpret_cast<page_map *>(page_map_phys);

        // Map page map recursively
        m_page_map_phys->entry(RECUR_MAP_INDEX) = page_map_entry(page_map_phys, true, true, false, false, false, false);

        // Map the kernel pages. This needs to be done "manually", i.e. without recursive mapping,
        // since paging isn't enabled yet but we need the mapping to activate paging

        uintptr_t virt_begin = reinterpret_cast<uintptr_t>(&KERNEL_VIRT_BEGIN);
        uintptr_t virt_end = reinterpret_cast<uintptr_t>(&KERNEL_VIRT_END);
        uintptr_t curr_phys = reinterpret_cast<uintptr_t>(&KERNEL_PHYS_BEGIN);

        // TODO: distinction into read-only and such
        // TODO: use large pages if possible?
        for(uintptr_t addr = virt_begin; addr < virt_end; addr += PAGE_SIZE, curr_phys += PAGE_SIZE) {
            this->map_pre_paging(addr, curr_phys);
        }

        // Re-initialize the physical memory manager to use virtual addressing
        // Set the new page directory
        set_page_directory(page_map_phys);

        // Free the (now useless) old page-directory page frames
        phys_mem_manager::instance().free_address(reinterpret_cast<uintptr_t>(&_pml4) - VIRT_OFFSET);
        phys_mem_manager::instance().free_address(reinterpret_cast<uintptr_t>(&_pdp) - VIRT_OFFSET);
        phys_mem_manager::instance().free_address(reinterpret_cast<uintptr_t>(&_pd) - VIRT_OFFSET);
        
        // Set the virtual memory range that the kernel can use for allocation
        m_vkernel_start = align_up(virt_end);
        m_vkernel_end = align_down(0xFFFFFFFFFFFFFFFF);
    }

    uintptr_t virt_mem_manager::map(uintptr_t virt, uintptr_t phys, bool kernel,
                        bool read_only, bool global) noexcept {
        size_t table = table_index(virt);
        size_t dir = dir_index(virt);
        size_t dir_ptr = dir_ptr_index(virt);
        size_t map = map_index(virt);

        // Check the top-level map
        if(!m_page_map->entry(map).is_present()) {
            // Reserve RAM for the page directory pointer and create the mapping
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
            m_page_map->entry(map) = page_map_entry(addr, true, true, !kernel, false, false, false);
        } else if(!m_page_map->entry(map).is_user()) {
            // Do something about this... users shouldn't map into kernel space
        }

        // Check the page-directory-pointer
        if(!m_page_dir_ptr->entry(map, dir_ptr).is_present()) {
            // Reserve RAM for the page directory and create the mapping
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
            m_page_dir_ptr->entry(map, dir_ptr) = page_dir_ptr_entry(addr, true, true, !kernel, false, false, false);
        }

        // Check the page-directory
        if(!m_page_dir->entry(map, dir_ptr, dir).is_present()) {
            // Reserve RAM for the page table and create the mapping
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
            m_page_dir->entry(map, dir_ptr, dir) = page_dir_entry(addr, true, true, !kernel, false, false, false);
        }

        // Add the table mapping
        m_page_table->entry(map, dir_ptr, dir, table) = page_table_entry(phys, true, !read_only, !kernel,
                false, false, false, global, false);
        
        return virt;
    }

    uintptr_t virt_mem_manager::map(uintptr_t virt, uintptr_t phys, size_t count, bool kernel,
                        bool read_only, bool global) noexcept {
        for(size_t curr_page = 0; curr_page < count; ++curr_page) {
            size_t table = table_index(virt + curr_page*PAGE_SIZE);
            size_t dir = dir_index(virt + curr_page*PAGE_SIZE);
            size_t dir_ptr = dir_ptr_index(virt + curr_page*PAGE_SIZE);
            size_t map = map_index(virt + curr_page*PAGE_SIZE);

            // Check the top-level map
            if(!m_page_map->entry(map).is_present()) {
                // Reserve RAM for the page directory pointer and create the mapping
                uintptr_t addr = phys_mem_manager::instance().alloc_any();
                m_page_map->entry(map) = page_map_entry(addr, true, true, !kernel, false, false, false);
            } else if(!m_page_map->entry(map).is_user()) {
                // Do something about this... users shouldn't map into kernel space
            }

            // Check the page-directory-pointer
            if(!m_page_dir_ptr->entry(map, dir_ptr).is_present()) {
                // Reserve RAM for the page directory and create the mapping
                uintptr_t addr = phys_mem_manager::instance().alloc_any();
                m_page_dir_ptr->entry(map, dir_ptr) = page_dir_ptr_entry(addr, true, true, !kernel, false, false, false);
            }

            // Check the page-directory
            if(!m_page_dir->entry(map, dir_ptr, dir).is_present()) {
                // Reserve RAM for the page table and create the mapping
                uintptr_t addr = phys_mem_manager::instance().alloc_any();
                m_page_dir->entry(map, dir_ptr, dir) = page_dir_entry(addr, true, true, !kernel, false, false, false);
            }

            // Add the table mapping
            m_page_table->entry(map, dir_ptr, dir, table) = page_table_entry(phys + curr_page*PAGE_SIZE,
                    true, !read_only, !kernel, false, false, false, global, false);
        }

        return virt;
    }
    uintptr_t virt_mem_manager::map_range(uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys,
                        bool kernel, bool read_only, bool global) noexcept {
        uintptr_t curr_virt = virt_start;
        while(curr_virt < virt_end) {
            size_t table = table_index(curr_virt);
            size_t dir = dir_index(curr_virt);
            size_t dir_ptr = dir_ptr_index(curr_virt);
            size_t map = map_index(curr_virt);

            // Check the top-level map
            if(!m_page_map->entry(map).is_present()) {
                // Reserve RAM for the page directory pointer and create the mapping
                uintptr_t addr = phys_mem_manager::instance().alloc_any();
                m_page_map->entry(map) = page_map_entry(addr, true, true, !kernel, false, false, false);
            } else if(!m_page_map->entry(map).is_user()) {
                // Do something about this... users shouldn't map into kernel space
            }

            // Check the page-directory-pointer
            if(!m_page_dir_ptr->entry(map, dir_ptr).is_present()) {
                // Reserve RAM for the page directory and create the mapping
                uintptr_t addr = phys_mem_manager::instance().alloc_any();
                m_page_dir_ptr->entry(map, dir_ptr) = page_dir_ptr_entry(addr, true, true, !kernel, false, false, false);
            }

            // Check the page-directory
            if(!m_page_dir->entry(map, dir_ptr, dir).is_present()) {
                // Reserve RAM for the page table and create the mapping
                uintptr_t addr = phys_mem_manager::instance().alloc_any();
                m_page_dir->entry(map, dir_ptr, dir) = page_dir_entry(addr, true, true, !kernel, false, false, false);
            }

            // Add the table mapping
            m_page_table->entry(map, dir_ptr, dir, table) = page_table_entry(phys, true, !read_only, !kernel,
                    false, false, false, global, false);
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
