#include "virt_mem.h"
#include "hal/memory/phys_mem.h"
#include "main/kernel.h"

// Comes from the linker
extern uintptr_t KERNEL_VIRT_OFFSET;
extern uintptr_t KERNEL_PHYS_BEGIN;
extern uintptr_t KERNEL_VIRT_BEGIN;
extern uintptr_t KERNEL_VIRT_END;

namespace hal {

    virt_mem_manager::virt_mem_manager() noexcept {
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
        return 0;
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