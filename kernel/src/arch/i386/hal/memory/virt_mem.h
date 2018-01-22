#ifndef DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_VIRT_MEM_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_VIRT_MEM_H_

#include <stdint.h>
#include <stddef.h>

#include "hal/memory/paging.h"

namespace hal {

    class virt_mem_manager {
    public:
        static constexpr size_t PAGE_SIZE = 4096;
    	
    	static constexpr size_t page_count(size_t bytes) noexcept {
    		return (bytes - 1) / virt_mem_manager::PAGE_SIZE + 1;
    	}
    	
    	static constexpr uintptr_t align_down(size_t addr) noexcept {
    		return (addr / virt_mem_manager::PAGE_SIZE) * virt_mem_manager::PAGE_SIZE;
    	}
    	
    	static constexpr uintptr_t align_up(size_t addr) noexcept {
    		return page_count(addr) * virt_mem_manager::PAGE_SIZE;
    	}
    	
    	constexpr uintptr_t vkernel_start() noexcept {
    		return m_vkernel_start;
    	}
    	
    	constexpr uintptr_t vkernel_end() noexcept {
    		return m_vkernel_start;
    	}

    private:
        static constexpr size_t dir_index(uintptr_t address) noexcept {
            return address / (PAGE_SIZE * page_table::ENTRIES);
        }
        
        static constexpr size_t table_index(uintptr_t address) noexcept {
            return (address % (PAGE_SIZE * page_table::ENTRIES)) / PAGE_SIZE;
        }

        static constexpr uintptr_t table_index(size_t dir, size_t table) noexcept {
            return dir * PAGE_SIZE * page_table::ENTRIES + table * PAGE_SIZE;
        }

        static void set_page_directory(uintptr_t phys_dir_addr);

        page_dir_entry *m_page_directory;
        page_table *m_tables;
        
        uintptr_t m_vkernel_start;
        uintptr_t m_vkernel_end;

        virt_mem_manager() noexcept;
    
    public:
        virt_mem_manager(const virt_mem_manager&) = delete;
        virt_mem_manager(virt_mem_manager&&) = delete;
        virt_mem_manager &operator=(const virt_mem_manager&) = delete;
        virt_mem_manager &operator=(virt_mem_manager&&) = delete;

        static virt_mem_manager &instance() noexcept;
        void init() noexcept;

        uintptr_t map(uintptr_t virt, uintptr_t phys, bool kernel, bool read_only, bool global) noexcept;
        uintptr_t map(uintptr_t virt, uintptr_t phys, size_t count, bool kernel,
                        bool read_only, bool global) noexcept;
        uintptr_t map_range(uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys,
                            bool kernel, bool read_only, bool global) noexcept;

        /*bool test_and_map(uintptr_t virt, uintptr_t phys, bool kernel, bool read_only, bool global) noexcept;
        bool test_and_map(uintptr_t virt, uintptr_t phys, size_t count, bool kernel,
                        bool read_only, bool global) noexcept;
        bool test_and_map_range(uintptr_t virt_start, uintptr_t virt_end, uintptr_t phys,
                            bool kernel, bool read_only, bool global) noexcept;*/

        // TODO: unmap!
    };

} // namespace hal

#endif //DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_VIRT_MEM_H_
