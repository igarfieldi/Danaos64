#ifndef DANAOS_KERNEL_HAL_MEMORY_PHYS_MEM_H_
#define DANAOS_KERNEL_HAL_MEMORY_PHYS_MEM_H_

#include <stdint.h>
#include <stddef.h>
#include "hal/util/bitmap.h"

namespace hal {

    class phy_mem_manager {
    public:
        static constexpr size_t PAGE_FRAME_SIZE = 4096;

    private:
        static constexpr size_t get_page_frame(uintptr_t address) noexcept {
            return address / PAGE_FRAME_SIZE;
        }
        
        static constexpr size_t get_page_frame(void *ptr) noexcept {
            return get_page_frame(reinterpret_cast<uintptr_t>(ptr));
        }

        phy_mem_manager() noexcept;

        size_t m_page_frame_count;

        util::bitmap<size_t> m_phys_bitmap;
    
    public:
        phy_mem_manager(const phy_mem_manager&) = delete;
        phy_mem_manager(phy_mem_manager&&) = delete;
        phy_mem_manager &operator=(const phy_mem_manager&&);
        phy_mem_manager &operator=(phy_mem_manager&&);

        static phy_mem_manager &instance() noexcept;

        void init(uintptr_t bitmap_address, size_t page_frames) noexcept;

        bool is_available_frame(size_t page_frame) const noexcept;
        bool is_available_frame(size_t page_frame, size_t frame_count) const noexcept;
        bool is_available_address(uintptr_t address) const noexcept;
        bool is_available_address(uintptr_t address, size_t frame_count) const noexcept;
        bool is_available_range(uintptr_t address, size_t bytes) const noexcept;
        
        void alloc_frame(size_t page_frame) noexcept;
        void alloc_frame(size_t page_frame, size_t frame_count) noexcept;
        void alloc_address(uintptr_t address) noexcept;
        void alloc_address(uintptr_t address, size_t frame_count) noexcept;
        void alloc_range(uintptr_t address, size_t bytes) noexcept;

        uintptr_t alloc_any() noexcept;
        uintptr_t alloc_any(size_t frame_count) noexcept;
        uintptr_t alloc_any_address(uintptr_t address_hint) noexcept;
        uintptr_t alloc_any_address(uintptr_t address_hint, size_t frame_count) noexcept;
        uintptr_t alloc_any_range(size_t bytes) noexcept;
        uintptr_t alloc_any_range(uintptr_t address_hint, size_t bytes) noexcept;

        bool test_and_alloc_frame(size_t page_frame) noexcept;
        bool test_and_alloc_frame(size_t page_frame, size_t frame_count) noexcept;
        bool test_and_alloc_address(uintptr_t address) noexcept;
        bool test_and_alloc_address(uintptr_t address, size_t frame_count) noexcept;
        bool test_and_alloc_range(uintptr_t address, size_t bytes) noexcept;
        
        void free_frame(size_t page_frame) noexcept;
        void free_frame(size_t page_frame, size_t frame_count) noexcept;
        void free_address(uintptr_t address) noexcept;
        void free_address(uintptr_t address, size_t frame_count) noexcept;
        void free_range(uintptr_t address, size_t bytes) noexcept;
        
        bool test_and_free_frame(size_t page_frame) noexcept;
        bool test_and_free_frame(size_t page_frame, size_t frame_count) noexcept;
        bool test_and_free_address(uintptr_t address) noexcept;
        bool test_and_free_address(uintptr_t address, size_t frame_count) noexcept;
        bool test_and_free_range(uintptr_t address, size_t bytes) noexcept;
    };

} // namespace hal

#endif //DANAOS_KERNEL_HAL_MEMORY_PHYS_MEM_H_