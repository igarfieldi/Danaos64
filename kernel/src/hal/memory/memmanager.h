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
    	uintptr_t m_vkernel_curr;
    	uintptr_t m_vkernel_alloced;
    	
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
    };

} // namespace hal

#endif //DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_
