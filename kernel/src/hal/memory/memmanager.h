#ifndef DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_
#define DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_

#include <stdint.h>
#include <stddef.h>
#include "hal/memory/memmap.h"
#include "hal/memory/phys_mem.h"
#include "main/kernel.h"
#include "hal/util/bitmap.h"

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
        memory_manager();
    
    public:
        memory_manager(const memory_manager&) = delete;
        memory_manager(memory_manager&&) = delete;
        memory_manager &operator=(const memory_manager&) = delete;
        memory_manager &operator=(memory_manager&&) = delete;

        static memory_manager &instance();

        template < class map_type >
        void init(map_type map) {
            using area_type = typename map_type::entry_type::area_type;

            // Compute how many page frames of physical RAM we have
            uint64_t highest_address = 0;
            for(auto &entry : map) {
                // While we're here mark memory below 1MBB as unavailable because it contains BIOS stuff we don't wanna overwrite
                if(entry.addr < 0x100000) {
                    entry.type = area_type::RESERVED;
                }

                kernel::m_console.print("Address: [], Length: {}, Type: {}\n",
                    entry.addr, entry.len, static_cast<uint32_t>(entry.type));

                if(entry.addr + entry.len > highest_address) {
                    highest_address = entry.addr + entry.len;
                }
            }
            size_t page_frame_count = highest_address / phy_mem_manager::PAGE_FRAME_SIZE;

            // TODO: also allow bitmap to go before the kernel!

            // Now find space where we can put our bitmap
            size_t bitmap_size = page_frame_count / sizeof(char);
            uintptr_t bitmap_address = 0;
            for(auto &entry : map) {
                // Has to be free and not come before the kernel
                if(entry.type == area_type::AVAILABLE &&
                            entry.addr >= reinterpret_cast<uintptr_t>(&KERNEL_PHYS_BEGIN)) {
                    // Check if we share the area with the kernel
                    if(entry.addr >= reinterpret_cast<uintptr_t>(&KERNEL_PHYS_END)) {
                        // Not sharing, check for size
                        if(entry.len >= bitmap_size) {
                            bitmap_address = entry.addr;
                            break;
                        }
                    } else {
                        // Sharing, need to make sure we got enough space left
                        uintptr_t end_addr = entry.addr + entry.len;
                        if(end_addr >= reinterpret_cast<uintptr_t>(&KERNEL_PHYS_END) &&
                                end_addr - reinterpret_cast<uintptr_t>(&KERNEL_PHYS_END) >= bitmap_size) {
                            // Enough space, all good
                            bitmap_address = reinterpret_cast<uintptr_t>(&KERNEL_PHYS_END);
                            break;
                        }
                    }
                }
            }

            // No address means no memory!
            if(bitmap_address == 0) {
                kernel::panic("Couldn't find space for the memory bitmap!");
            }

            // Initialize the physical memory manager
            phy_mem_manager::instance().init(bitmap_address, page_frame_count);
            
            // Mark kernel frames and everything below 1MB as used
            phy_mem_manager::instance().alloc_range(0, reinterpret_cast<uintptr_t>(&KERNEL_PHYS_END));
            // Mark the RAM areas
            for(auto &entry : map) {
                if(entry.type != area_type::AVAILABLE) {
                    phy_mem_manager::instance().alloc_range(entry.addr, entry.len);
                }
            }

            

            kernel::m_console.print("Bitmap location: []\n", bitmap_address);
            kernel::m_console.print("Max: {}\n", std::numeric_limits<unsigned long long>::max());
            kernel::m_console.print("Kernel addresses: [] [] [] [] [] [] [] []\n",
                        &KERNEL_PHYS_BEGIN, &KERNEL_PHYS_END,
                        &KERNEL_PHYS_CODE_BEGIN, &KERNEL_PHYS_CODE_END,
                        &KERNEL_PHYS_DATA_BEGIN, &KERNEL_PHYS_DATA_END,
                        &KERNEL_PHYS_RODATA_BEGIN, &KERNEL_PHYS_RODATA_END);
            phy_mem_manager::instance().alloc_address(0x115000);
            kernel::m_console.print("Alloc: []\n", phy_mem_manager::instance().alloc_any(3));
        }
    };

} // namespace hal

#endif //DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_