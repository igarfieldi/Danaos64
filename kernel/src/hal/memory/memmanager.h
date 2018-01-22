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

        template < class map_type >
        void init(map_type map) noexcept {
            using area_type = typename map_type::entry_type::area_type;

			// Do the first (provisional) initialization of the pmm with the bitmap alloc'ed in the binary
			(void) hal::phys_mem_manager::instance();
            
            // Mark the non-accessible page frames and count the highest available page frame
            uint64_t highest_address = 0;
            for(auto &entry : map) {
                // While we're here mark memory below 1MBB as unavailable because it contains BIOS stuff we don't wanna overwrite
                if(entry.type != area_type::AVAILABLE) {
                	hal::phys_mem_manager::instance().alloc_range(entry.addr, entry.len);
                } else if(entry.addr + entry.len > highest_address) {
                    highest_address = entry.addr + entry.len;
                }
                

                kernel::m_console.print("Address: [], Length: {}, Type: {}\n",
                    entry.addr, entry.len, static_cast<uint32_t>(entry.type));
            }
            size_t page_frame_count = highest_address / phys_mem_manager::PAGE_FRAME_SIZE;

			/*
            // Now find space where we can put our bitmap
            size_t bitmap_size = page_frame_count / CHAR_BIT;
            uintptr_t bitmap_address = 0;
            for(auto &entry : map) {
                // TODO: ensure that we stay within the addressable memory range!
                // Has to be free and not come before the kernel
                if(entry.type == area_type::AVAILABLE &&
                            entry.addr <= kernel_phys_begin &&
                            entry.addr + entry.len > kernel_phys_end) {
                    // Ensure enough space when considering the kernel
                    if((kernel_phys_end < entry.addr) ||
                                (entry.addr + entry.len - kernel_phys_end >= bitmap_size)) {
                        uintptr_t after_kernel = math::max<uintptr_t>(entry.addr, kernel_phys_end);
                        size_t entry_length = entry.addr + entry.len - after_kernel;

                        // Now check if we overlap with the memory map
                        if(after_kernel > map.address() + map.size()) {
                            // No problem, check for length
                            if(entry_length >= bitmap_size) {
                                bitmap_address = after_kernel;
                                break;
                            }
                        } else {
                            // Possible problem, check if we can fit it before or after the memory map
                            if((after_kernel + bitmap_size < map.address()) && (entry_length >= bitmap_size)) {
                                bitmap_address = after_kernel;
                                break;
                            } else if(after_kernel + entry_length - (map.address() + map.size()) >= bitmap_size) {
                                bitmap_address = map.address() + map.size();
                                break;
                            }
                        }
                    }
                }
            }*/
            
			// Mark kernel frames and everything below 1MB as used
			hal::phys_mem_manager::instance().alloc_range(0, reinterpret_cast<uintptr_t>(&KERNEL_PHYS_END));
            
            // Check if we need a larger bitmap
            if(page_frame_count > hal::phys_mem_manager::instance().page_frame_count()) {
            	kernel::m_console.print("Warning: more RAM installed than currently handleable by PMM!\n");
            }

			/*
            // No address means no memory!
            if(bitmap_address == 0) {
                kernel::panic("Couldn't find space for the memory bitmap!");
            }

            // Initialize the physical memory manager
            phys_mem_manager::instance().init(bitmap_address, page_frame_count);
            
            // Mark kernel frames and everything below 1MB as used
            phys_mem_manager::instance().alloc_range(0, kernel_phys_end);
            // Mark the RAM areas
            for(auto &entry : map) {
                if(entry.type != area_type::AVAILABLE) {
                    phys_mem_manager::instance().alloc_range(entry.addr, entry.len);
                }
            }*/

            kernel::m_console.print("Kernel: [] - []\n", reinterpret_cast<uintptr_t>(&KERNEL_PHYS_BEGIN),
                                reinterpret_cast<uintptr_t>(&KERNEL_PHYS_END));
            //kernel::m_console.print("Bitmap: [] - []\n", bitmap_address, bitmap_address + bitmap_size);

			
            virt_mem_manager::instance().init();
            m_vkernel_curr = virt_mem_manager::instance().vkernel_start();
            m_vkernel_alloced = virt_mem_manager::instance().vkernel_start();
        }
        
        uintptr_t kernel_alloc_pages(uintptr_t phys, size_t size) noexcept;
        
        template < class T >
        T *kernel_alloc_pages(T *phys) noexcept {
        	return reinterpret_cast<T*>(kernel_alloc_pages(reinterpret_cast<uintptr_t>(phys), sizeof(T)));
        }
    };

} // namespace hal

#endif //DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_
