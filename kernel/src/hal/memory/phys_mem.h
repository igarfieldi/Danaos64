#ifndef DANAOS_KERNEL_HAL_MEMORY_PHYS_MEM_H_
#define DANAOS_KERNEL_HAL_MEMORY_PHYS_MEM_H_

#include <stdint.h>
#include <stddef.h>
#include "hal/util/bitmap.h"
#include "main/kernel.h"

extern size_t _phys_bitmap;
extern uintptr_t KERNEL_VIRT_OFFSET;
extern uintptr_t KERNEL_PHYS_BEGIN;
extern uintptr_t KERNEL_PHYS_END;

namespace hal {

    class phys_mem_manager {
    public:
        static constexpr size_t PAGE_FRAME_SIZE = 4096;
    	
    	static constexpr size_t page_count(size_t bytes) noexcept {
    		return (bytes - 1) / PAGE_FRAME_SIZE + 1;
    	}

    private:
    	static constexpr size_t INIT_BITMAP_SIZE = 0x20000;
    
        static constexpr size_t get_page_frame(uintptr_t address) noexcept {
            return address / PAGE_FRAME_SIZE;
        }
        
        static constexpr size_t get_page_frame(void *ptr) noexcept {
            return get_page_frame(reinterpret_cast<uintptr_t>(ptr));
        }

        static constexpr size_t round_page_up(uintptr_t address) noexcept {
            return 1 + (address - 1) / PAGE_FRAME_SIZE;
        }

        phys_mem_manager() noexcept;

        util::bitmap<size_t> m_phys_bitmap;
    
    public:
        phys_mem_manager(const phys_mem_manager&) = delete;
        phys_mem_manager(phys_mem_manager&&) = delete;
        phys_mem_manager &operator=(const phys_mem_manager&&);
        phys_mem_manager &operator=(phys_mem_manager&&);

        static phys_mem_manager &instance() noexcept;

        template < class map_type >
        void init(map_type map) noexcept {
        	using area_type = typename map_type::entry_type::area_type;
            
            kernel::m_console.print("Initializing PMM...\n");

			// Do the first (provisional) initialization of the pmm with the bitmap alloc'ed in the binary
			uintptr_t virt = reinterpret_cast<uintptr_t>(&_phys_bitmap);
    		uintptr_t phys = virt - reinterpret_cast<uintptr_t>(&KERNEL_VIRT_OFFSET);
    		size_t init_page_frames = 0x20000;
		    m_phys_bitmap = util::bitmap<size_t>(virt, init_page_frames);
		    m_phys_bitmap.clear();
		    // Allocate whatever space the bitmap needs
		    this->alloc_range(phys, init_page_frames / CHAR_BIT);
            
            // Mark the non-accessible page frames and count the highest available page frame
            uint64_t highest_address = 0;
            for(auto &entry : map) {
                // While we're here mark memory below 1MBB as unavailable because it contains BIOS stuff we don't wanna overwrite
                if(entry.type != area_type::AVAILABLE) {
                	hal::phys_mem_manager::instance().alloc_range(entry.addr, entry.len);
                } else if(entry.addr + entry.len > highest_address) {
                    highest_address = entry.addr + entry.len;
                }
                
            }
            size_t page_frame_count = highest_address / phys_mem_manager::PAGE_FRAME_SIZE;
            
            kernel::m_console.print("  RAM          : [] MB\n", highest_address / 0x100000);
            
			// Mark kernel frames and everything below 1MB as used
			hal::phys_mem_manager::instance().alloc_range(0, reinterpret_cast<uintptr_t>(&KERNEL_PHYS_END));
            
            // Check if we need a larger bitmap
            if(page_frame_count > hal::phys_mem_manager::instance().page_frame_count()) {
            	kernel::m_console.print("Warning: more RAM installed than currently handleable by PMM!\n");
            }

            kernel::m_console.print("  Kernel phys  : [] - []\n", reinterpret_cast<uintptr_t>(&KERNEL_PHYS_BEGIN),
                                reinterpret_cast<uintptr_t>(&KERNEL_PHYS_END));
        }
        
        constexpr size_t page_frame_count() const noexcept {
        	return m_phys_bitmap.bits();
        }

        bool is_available_frame(size_t page_frame) const noexcept;
        bool is_available_frame(size_t page_frame, size_t frame_count) const noexcept;
        bool is_available_address(uintptr_t address) const noexcept;
        bool is_available_address(uintptr_t address, size_t frame_count) const noexcept;
        bool is_available_range(uintptr_t address, size_t bytes) const noexcept;
        
        uintptr_t alloc_frame(size_t page_frame) noexcept;
        uintptr_t alloc_frame(size_t page_frame, size_t frame_count) noexcept;
        uintptr_t alloc_address(uintptr_t address) noexcept;
        uintptr_t alloc_address(uintptr_t address, size_t frame_count) noexcept;
        uintptr_t alloc_range(uintptr_t address, size_t bytes) noexcept;

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
