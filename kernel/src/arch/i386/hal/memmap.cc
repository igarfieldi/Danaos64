#include "memmap.h"

namespace hal {

    multiboot_memmap_iterator::multiboot_memmap_iterator(const multiboot_tag_mmap &mmap) :
                m_mmap(&mmap), m_index(0), m_size((mmap.size - sizeof(multiboot_tag)) / mmap.entry_size) {
    }

    const memmap_entry &multiboot_memmap_iterator::operator*() {
        return m_mmap->entries[m_index];
    }

    const memmap_entry &multiboot_memmap_iterator::operator[](size_t i) {
        return m_mmap->entries[i];
    }

    multiboot_memmap_iterator &multiboot_memmap_iterator::operator++() {
        ++m_index;
        return *this;
    }

    multiboot_memmap_iterator &multiboot_memmap_iterator::operator--() {
        --m_index;
        return *this;
    }

    multiboot_memmap_iterator multiboot_memmap_iterator::operator++(int) {
        multiboot_memmap_iterator old(*this);
        ++m_index;
        return old;
    }

    multiboot_memmap_iterator multiboot_memmap_iterator::operator--(int) {
        multiboot_memmap_iterator old(*this);
        --m_index;
        return old;
    }

    multiboot_memmap_iterator &multiboot_memmap_iterator::operator+=(size_t i) {
        m_index += i;
        return *this;
    }

    multiboot_memmap_iterator &multiboot_memmap_iterator::operator-=(size_t i) {
        m_index -= i;
        return *this;
    }

    const memmap_entry *multiboot_memmap_iterator::begin() {
        return &m_mmap->entries[0];
    }

    const memmap_entry *multiboot_memmap_iterator::cbegin() const {
        return &m_mmap->entries[0];
    }

    const memmap_entry *multiboot_memmap_iterator::end() {
        return &m_mmap->entries[m_size];
    }

    const memmap_entry *multiboot_memmap_iterator::cend() const {
        return &m_mmap->entries[m_size];
    }

    const memmap_entry *begin(multiboot_memmap_iterator &iter) {
        return iter.begin();
    }

    const memmap_entry *cbegin(const multiboot_memmap_iterator &iter) {
        return iter.cbegin();
    }

    const memmap_entry *end(multiboot_memmap_iterator &iter) {
        return iter.end();
    }

    const memmap_entry *cend(const multiboot_memmap_iterator &iter) {
        return iter.cend();
    }

} // namespace hal