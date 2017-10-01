#ifndef DANAOS_KERNEL_HAL_MEMORY_MEMMAP_H_
#define DANAOS_KERNEL_HAL_MEMORY_MEMMAP_H_

#include <stdint.h>
#include <stddef.h>
#include "boot/multiboot2.h"
#include "hal/memory/memmap_iterator.h"

namespace hal {

    class multiboot_memmap {
    public:
        using entry_type = multiboot_mmap_entry;
        using iterator = memmap_iterator<entry_type>;
        using const_iterator = const_memmap_iterator<entry_type>;

    private:
        multiboot_tag_mmap &m_map;

    public:
        multiboot_memmap(multiboot_tag_mmap &map);

        entry_type &operator[](size_t i);
        const entry_type &operator[](size_t i) const;

        uintptr_t address() const;
        size_t size() const;
        size_t entries() const;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;
    };

}

#endif //DANAOS_KERNEL_HAL_MEMORY_MEMMAP_H_