#include "memmap.h"

namespace hal {
    
    multiboot_memmap::multiboot_memmap(multiboot_tag_mmap &map) : m_map(map) {
    }

    multiboot_memmap::entry_type &multiboot_memmap::operator[](size_t i) {
        return m_map.entries[i];
    }

    const multiboot_memmap::entry_type &multiboot_memmap::operator[](size_t i) const {
        return m_map.entries[i];
    }

    multiboot_memmap::iterator multiboot_memmap::begin() {
        return iterator(m_map.entries[0]);
    }

    multiboot_memmap::iterator multiboot_memmap::end() {
        return iterator(m_map.entries[(m_map.size - sizeof(multiboot_tag_mmap)) / m_map.entry_size]);
    }
    
    multiboot_memmap::const_iterator multiboot_memmap::begin() const {
        return const_iterator(m_map.entries[0]);
    }
    
    multiboot_memmap::const_iterator multiboot_memmap::end() const {
        return const_iterator(m_map.entries[(m_map.size - sizeof(multiboot_tag_mmap)) / m_map.entry_size]);
    }
    
    multiboot_memmap::const_iterator multiboot_memmap::cbegin() const {
        return const_iterator(m_map.entries[0]);
    }
    
    multiboot_memmap::const_iterator multiboot_memmap::cend() const {
        return const_iterator(m_map.entries[(m_map.size - sizeof(multiboot_tag_mmap)) / m_map.entry_size]);
    }

}