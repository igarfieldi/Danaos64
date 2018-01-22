#ifndef DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_VIRT_MEM_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_VIRT_MEM_H_

#include <stdint.h>
#include <stddef.h>

#include "hal/memory/paging.h"

namespace hal {

    class virt_mem_manager {
    private:
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
