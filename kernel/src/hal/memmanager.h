#ifndef DANAOS_KERNEL_HAL_MEMMANAGER_H_
#define DANAOS_KERNEL_HAL_MEMMANAGER_H_

#include <stdint.h>
#include <stddef.h>
#include "hal/memmap.h"
#include "main/kernel.h"

namespace hal {

    class memory_manager {
    private:
        size_t m_page_frame_count;
        memory_manager();
    
    public:
        memory_manager(const memory_manager&) = delete;
        memory_manager(memory_manager&&) = delete;
        memory_manager &operator=(const memory_manager&) = delete;
        memory_manager &operator=(memory_manager&&) = delete;

        static memory_manager &instance();

        template < class memmap_iterator >
        void init(memmap_iterator iterator) {
            for(const memmap_entry &entry : iterator) {
                kernel::m_console.print("Address: [], Length: {}, Type: {}\n",
                    entry.addr, entry.len, entry.type);
            }
        }
    };

} // namespace hal

#endif //DANAOS_KERNEL_HAL_MEMMANAGER_H_