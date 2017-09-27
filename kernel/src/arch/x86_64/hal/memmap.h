#ifndef DANAOS_KERNEL_ARCH_I386_HAL_MEMMAP_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_MEMMAP_H_

#include <stdint.h>
#include <stddef.h>
#include "boot/multiboot2.h"

namespace hal {

    using memmap_entry = multiboot_mmap_entry;

    class multiboot_memmap_iterator {
    private:
        const multiboot_tag_mmap *m_mmap;
        size_t m_index;
        size_t m_size;

    public:
        multiboot_memmap_iterator(const multiboot_tag_mmap &mmap);

        const memmap_entry &operator*();
        const memmap_entry &operator[](size_t i);

        multiboot_memmap_iterator &operator++();
        multiboot_memmap_iterator &operator--();
        multiboot_memmap_iterator operator++(int);
        multiboot_memmap_iterator operator--(int);

        multiboot_memmap_iterator &operator+=(size_t i);
        multiboot_memmap_iterator &operator-=(size_t i);

        const memmap_entry *begin();
        const memmap_entry *cbegin() const;
        const memmap_entry *end();
        const memmap_entry *cend() const;
    };
    
    const memmap_entry *begin(multiboot_memmap_iterator &iter);
    const memmap_entry *cbegin(const multiboot_memmap_iterator &iter);
    const memmap_entry *end(multiboot_memmap_iterator &iter);
    const memmap_entry *cend(const multiboot_memmap_iterator &iter);

} // namespace hal

#endif //DANAOS_KERNEL_ARCH_I386_HAL_MEMMAP_H_