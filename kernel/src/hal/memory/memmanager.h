#ifndef DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_
#define DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_

#include <stdint.h>
#include <stddef.h>
#include "hal/memory/memmap.h"
#include "hal/memory/phys_mem.h"
#include "hal/memory/virt_mem.h"
#include "main/kernel.h"
#include "hal/util/bitmap.h"
#include "libk/math.h"
#include "libk/new.h"

extern uintptr_t KERNEL_PHYS_BEGIN;
extern uintptr_t KERNEL_PHYS_CODE_BEGIN;
extern uintptr_t KERNEL_PHYS_CODE_END;
extern uintptr_t KERNEL_PHYS_DATA_BEGIN;
extern uintptr_t KERNEL_PHYS_DATA_END;
extern uintptr_t KERNEL_PHYS_RODATA_BEGIN;
extern uintptr_t KERNEL_PHYS_RODATA_END;
extern uintptr_t KERNEL_PHYS_END;

namespace hal {

    class memory_manager {
    private:
        uintptr_t m_vheap_start;
        uintptr_t m_vheap_end;
        uintptr_t m_vheap_curr;
        uintptr_t m_vheap_alloced;

        static constexpr uintptr_t align(uintptr_t addr, uintptr_t alignment) noexcept {
            return ((addr - 1) / alignment + 1) * alignment;
        }
    	
        memory_manager() noexcept;
    
    public:
        memory_manager(const memory_manager&) = delete;
        memory_manager(memory_manager&&) = delete;
        memory_manager &operator=(const memory_manager&) = delete;
        memory_manager &operator=(memory_manager&&) = delete;

        static memory_manager &instance() noexcept;

        void init() noexcept;
        
        uintptr_t kernel_alloc_pages(uintptr_t phys, size_t size) noexcept;
        
        template < class T >
        T *kernel_alloc_pages(T *phys) noexcept {
        	return reinterpret_cast<T*>(kernel_alloc_pages(reinterpret_cast<uintptr_t>(phys), sizeof(T)));
        }

        template < class T >
        T *kernel_malloc(size_t count) noexcept {
            if(count == 0) {
                return nullptr;
            }

            size_t alignment = alignof(T);
            // Objects need to be aligned, so do it
            uintptr_t ret_addr = align(m_vheap_curr, alignment);
            uintptr_t vend = ret_addr + sizeof(T) * alignment;
            size_t bytes = vend - m_vheap_curr;
            
            if(bytes == 0) {
                return nullptr;
            }


            if(m_vheap_curr + bytes >= m_vheap_alloced) {
                // Allocate additional page(s)
                size_t ppages_needed = phys_mem_manager::page_count(bytes - (m_vheap_alloced - m_vheap_curr));
                for(size_t frame = 0; frame < ppages_needed; ++frame) {
                    uintptr_t phys = phys_mem_manager::instance().alloc_any();
                    virt_mem_manager::instance().map(m_vheap_alloced, phys, true, false, false);
                    m_vheap_alloced += phys_mem_manager::PAGE_FRAME_SIZE;
                }
            }

            return reinterpret_cast<T*>(ret_addr);
        }

        template < class T, class... Args >
        T *kernel_new(Args &&...args) noexcept {
            return new (kernel_malloc<T>(1)) T(static_cast<T&&>(args)...);
        }
    };

} // namespace hal

#endif //DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_
