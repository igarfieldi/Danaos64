#ifndef DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_
#define DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_

#include <stdint.h>
#include <stddef.h>
#include "hal/memory/memmap.h"
#include "main/kernel.h"

namespace hal {

    class memory_manager {
    private:
        static constexpr size_t PAGE_SIZE = 4096;

        size_t m_highest_page;
        size_t m_page_frame_count;

        template < class map_type >
        static typename map_type::entry_type *find_area(map_type map, size_t bytes) {
            using area_type = typename map_type::entry_type::area_type;
            
            for(auto &entry : map) {
                if(entry.type == area_type::AVAILABLE && entry.len >= bytes) {
                    return &entry;
                }
            }
            return nullptr;
        }

        memory_manager();
    
    public:
        memory_manager(const memory_manager&) = delete;
        memory_manager(memory_manager&&) = delete;
        memory_manager &operator=(const memory_manager&) = delete;
        memory_manager &operator=(memory_manager&&) = delete;

        static memory_manager &instance();

        template < class map_type >
        void init(map_type map) {
            m_page_frame_count = 0;

            // Compute how many page frames of physical RAM we have
            uintptr_t highest_address = 0;
            for(const auto &entry : map) {
                kernel::m_console.print("Address: [], Length: {}, Type: {}\n",
                    entry.addr, entry.len, static_cast<uint32_t>(entry.type));

                if(entry.addr + entry.len > highest_address) {
                    highest_address = entry.addr + entry.len;
                }
            }
            m_page_frame_count = highest_address / PAGE_SIZE;

            // Now find space where we can put our bitmap
            size_t bitmap_size = m_page_frame_count / sizeof(char);
            auto bitmap_entry = memory_manager::find_area(map, bitmap_size);
            if(bitmap_entry == nullptr) {
                kernel::panic("Couldn't find space for the memory bitmap!");
            }
        }
    };

} // namespace hal

#endif //DANAOS_KERNEL_HAL_MEMORY_MEMMANAGER_H_