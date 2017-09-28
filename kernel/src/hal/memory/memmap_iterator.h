#ifndef DANAOS_KERNEL_HAL_MEMORY_MEMMAP_ITERATOR_H_
#define DANAOS_KERNEL_HAL_MEMORY_MEMMAP_ITERATOR_H_

namespace hal {

    template < class E >
    class memmap_iterator {
    public:
        using entry_type = E;
        
    private:
        entry_type *m_ptr;

    public:
        memmap_iterator(entry_type &entry) : m_ptr(&entry) {
        }

        entry_type &operator*() {
            return *m_ptr;
        }

        entry_type *operator->() {
            return m_ptr;
        }

        entry_type &operator[](size_t i) {
            return m_ptr[i];
        }

        bool operator==(const memmap_iterator &iter) {
            return iter.m_ptr == this->m_ptr;
        }

        bool operator!=(const memmap_iterator &iter) {
            return iter.m_ptr != this->m_ptr;
        }

        const entry_type &operator*() const {
            return *m_ptr;
        }

        const entry_type *operator->() const {
            return m_ptr;
        }

        const entry_type &operator[](size_t i) const {
            return m_ptr[i];
        }

        memmap_iterator &operator++() {
            ++m_ptr;
            return *this;
        }

        memmap_iterator &operator--() {
            --m_ptr;
            return *this;
        }
        
        memmap_iterator operator++(int) {
            memmap_iterator old(*this);
            ++m_ptr;
            return old;
        }
        
        memmap_iterator operator--(int) {
            memmap_iterator old(*this);
            --m_ptr;
            return old;
        }

        memmap_iterator &operator+=(size_t i) {
            m_ptr += i;
            return *this;
        }
        
        memmap_iterator &operator-=(size_t i) {
            m_ptr -= i;
            return *this;
        }
    };

    template < class E >
    class const_memmap_iterator {
    public:
        using entry_type = E;
        
    private:
        const entry_type *m_ptr;

    public:
        const_memmap_iterator(entry_type &entry) : m_ptr(&entry) {
        }

        const entry_type &operator*() const {
            return *m_ptr;
        }

        const entry_type *operator->() const {
            return m_ptr;
        }

        const entry_type &operator[](size_t i) const {
            return m_ptr[i];
        }

        const_memmap_iterator &operator++() {
            ++m_ptr;
            return *this;
        }

        const_memmap_iterator &operator--() {
            --m_ptr;
            return *this;
        }
        
        const_memmap_iterator operator++(int) {
            const_memmap_iterator old(*this);
            ++m_ptr;
            return old;
        }
        
        const_memmap_iterator operator--(int) {
            const_memmap_iterator old(*this);
            --m_ptr;
            return old;
        }

        const_memmap_iterator &operator+=(size_t i) {
            m_ptr += i;
            return *this;
        }
        
        const_memmap_iterator &operator-=(size_t i) {
            m_ptr -= i;
            return *this;
        }
    };
} // namespace hal

#endif //DANAOS_KERNEL_HAL_MEMORY_MEMMAP_ITERATOR_H_