#include "memmanager.h"

namespace hal {

    memory_manager::memory_manager() noexcept : m_vkernel_curr(0), m_vkernel_alloced(0) {
    }

    memory_manager &memory_manager::instance() noexcept {
        static memory_manager inst;
        return inst;
    }
    
    void memory_manager::init() noexcept {
        m_vkernel_curr = virt_mem_manager::instance().vkernel_start();
        m_vkernel_alloced = virt_mem_manager::instance().vkernel_start();
    }
    
    uintptr_t memory_manager::kernel_alloc_pages(uintptr_t phys, size_t size) noexcept {
		// This fragments the kernel vm on the level of pages!
		
		uintptr_t aligned_phys = virt_mem_manager::align_down(phys);
		uintptr_t addr_diff = phys - aligned_phys;
		size += addr_diff;
		
		if(size == 0) {
			return 0;
		}
		
		size_t pages = virt_mem_manager::page_count(size);
		
		if(m_vkernel_alloced + virt_mem_manager::PAGE_SIZE * pages > virt_mem_manager::instance().vkernel_end()) {
			// Out of kernel memory
			kernel::panic("Out of kernel memory!");
		}
		uintptr_t ret_addr = m_vkernel_alloced + addr_diff;
		// Map the requested pages
		virt_mem_manager::instance().map(m_vkernel_alloced, aligned_phys, pages, true, false, false);
		// Move the allocation mark to the latest allocated page
		m_vkernel_alloced += virt_mem_manager::PAGE_SIZE * pages;
		m_vkernel_curr = m_vkernel_alloced;
		
		return ret_addr;
	}
}
