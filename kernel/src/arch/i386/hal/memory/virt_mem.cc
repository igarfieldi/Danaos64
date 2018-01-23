#include "virt_mem.h"
#include "hal/memory/phys_mem.h"
#include "main/kernel.h"
#include "libk/string.h"

// Comes from the linker
extern uintptr_t KERNEL_VIRT_OFFSET;
extern uintptr_t KERNEL_PHYS_BEGIN;
extern uintptr_t KERNEL_VIRT_BEGIN;
extern uintptr_t KERNEL_VIRT_END;
extern hal::page_dir_entry _page_directory;

namespace hal {

    void virt_mem_manager::set_page_directory(uintptr_t phys_dir_addr) {
        __asm__ volatile(
            "movl   %0, %%cr3"
            : 
            : "r"(phys_dir_addr)
            :
        );
    }
    
    void virt_mem_manager::map_pre_paging(uintptr_t virt, uintptr_t phys) {
        // TODO: distinction into read-only and such
        // TODO: use large pages if possible?
        size_t dir = dir_index(virt);
        size_t table = table_index(virt);

        // Add the mappings to the (physical) page directory and page table
        if(!m_page_dir_phys->entry(dir).is_present()) {
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
		    // Zero out the page
		    std::memset(reinterpret_cast<void*>(addr), 0, phys_mem_manager::PAGE_FRAME_SIZE);
            m_page_dir_phys->entry(dir) = page_dir_entry(addr, true, true, true, false, false);
        }

        // TODO: volatile?
        page_table_entry *pt = reinterpret_cast< page_table_entry *>(m_page_dir_phys->entry(dir).get_table_addr());
        pt[table] = page_table_entry(phys, true, true, true, true, false, false);
    }

    virt_mem_manager::virt_mem_manager() noexcept : 
            VIRT_OFFSET(reinterpret_cast<uintptr_t>(&KERNEL_VIRT_OFFSET)),
            m_page_dir_phys(nullptr),
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
        uintptr_t page_dir_phys = phys_mem_manager::instance().alloc_any();
        m_page_dir_phys = reinterpret_cast<page_dir *>(page_dir_phys);
        // Identity map to allow access
        this->map_pre_paging(page_dir_phys, page_dir_phys);
        // Zero out the page
        std::memset(reinterpret_cast<void*>(page_dir_phys), 0, phys_mem_manager::PAGE_FRAME_SIZE);

        // Map page map recursively
        m_page_dir_phys->entry(RECUR_DIR_INDEX) = page_dir_entry(page_dir_phys, true, true, false, false, false);

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
        set_page_directory(page_dir_phys);

        // Free the (now useless) old page-directory page frames
        phys_mem_manager::instance().free_address(reinterpret_cast<uintptr_t>(&_page_directory) - VIRT_OFFSET);
        
        // Set the virtual memory range that the kernel can use for allocation
        m_vkernel_start = align_up(reinterpret_cast<uintptr_t>(&KERNEL_VIRT_END));
        m_vkernel_end = align_down(0xFFFFFFFF);
        
    }

    uintptr_t virt_mem_manager::map(uintptr_t virt, uintptr_t phys, bool kernel,
                        bool read_only, bool global) noexcept {
                        
		size_t table = table_index(virt);
        size_t dir = dir_index(virt);

        // Check the top-level map
        if(!m_page_dir->entry(dir).is_present()) {
            // Reserve RAM for the page directory pointer and create the mapping
            uintptr_t addr = phys_mem_manager::instance().alloc_any();
		    // Zero out the page
		    std::memset(reinterpret_cast<void*>(addr), 0, phys_mem_manager::PAGE_FRAME_SIZE);
            m_page_dir->entry(dir) = page_dir_entry(addr, true, true, !kernel, false, false);
        } else if(!m_page_dir->entry(dir).is_user() && !kernel) {
            // Do something about this... users shouldn't map into kernel space
        }

        // Add the table mapping
        m_page_table->entry(dir, table) = page_table_entry(phys, true, !read_only, !kernel,
                false, false, global);
        return virt;
    }

    uintptr_t virt_mem_manager::map(uintptr_t virt, uintptr_t phys, size_t count, bool kernel,
                        bool read_only, bool global) noexcept {
        for(size_t curr_page = 0; curr_page < count; ++curr_page) {
        	size_t table = table_index(virt + curr_page*PAGE_SIZE);
		    size_t dir = dir_index(virt + curr_page*PAGE_SIZE);

		    // Check the top-level map
		    if(!m_page_dir->entry(dir).is_present()) {
		        // Reserve RAM for the page directory pointer and create the mapping
		        uintptr_t addr = phys_mem_manager::instance().alloc_any();
				// Zero out the page
				std::memset(reinterpret_cast<void*>(addr), 0, phys_mem_manager::PAGE_FRAME_SIZE);
		        m_page_dir->entry(dir) = page_dir_entry(addr, true, true, !kernel, false, false);
		    } else if(!m_page_dir->entry(dir).is_user() && !kernel) {
		        // Do something about this... users shouldn't map into kernel space
		    }

		    // Add the table mapping
		    m_page_table->entry(dir, table) = page_table_entry(phys + curr_page*PAGE_SIZE, true, !read_only, !kernel,
		            false, false, global);
        }

        return virt;
    }
    uintptr_t virt_mem_manager::map_range(uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys,
                        bool kernel, bool read_only, bool global) noexcept {
        uintptr_t curr_virt = virt_start;
        while(curr_virt < virt_end) {
        	size_t table = table_index(curr_virt);
		    size_t dir = dir_index(curr_virt);

		    // Check the top-level map
		    if(!m_page_dir->entry(dir).is_present()) {
		        // Reserve RAM for the page directory pointer and create the mapping
		        uintptr_t addr = phys_mem_manager::instance().alloc_any();
				// Zero out the page
				std::memset(reinterpret_cast<void*>(addr), 0, phys_mem_manager::PAGE_FRAME_SIZE);
		        m_page_dir->entry(dir) = page_dir_entry(addr, true, true, !kernel, false, false);
		    } else if(!m_page_dir->entry(dir).is_user() && !kernel) {
		        // Do something about this... users shouldn't map into kernel space
		    }

		    // Add the table mapping
		    m_page_table->entry(dir, table) = page_table_entry(phys, true, !read_only, !kernel,
		            false, false, global);
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
