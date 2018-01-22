#ifndef DANAOS_KERNEL_HAL_UTIL_BITMAP_H_
#define DANAOS_KERNEL_HAL_UTIL_BITMAP_H_

#include <stdint.h>
#include <stddef.h>
#include "libk/numeric_limits.h"
//#include <limits.h>

namespace util {

    template < class T >
    constexpr T get_mask(size_t pos, size_t length) {
        return ((T{1} << length) - T{1}) << pos;
    }

    template < class T >
    constexpr T set_bits(T word, size_t pos, size_t length, T val) {
        return (word & ~get_mask<T>(pos, length)) | (val & get_mask<T>(pos, length));
    }

    template < class T >
    constexpr T set_bit(T word, size_t pos, bool val) {
        return (word & ~get_mask<T>(pos, 1)) | (-val & get_mask<T>(pos, 1));
    }

    template < class T >
    constexpr T get_bits(T word, size_t pos, size_t length) {
        return word & get_mask<T>(pos, length);
    }

    template < class T >
    constexpr T get_bit(T word, size_t pos) {
        return word & get_mask<T>(pos, 1);
    }

    // TODO: specialization for word-size so we can use special CPU instructions!
    template < class B >
    class bitmap {
    public:
        using block_type = B;
        static constexpr size_t BITS_PER_BLOCK = sizeof(block_type) * CHAR_BIT;
        static constexpr size_t npos = std::numeric_limits<size_t>::max();

    private:
        size_t m_bits;
        size_t m_blocks;
        block_type *m_ptr;

        static constexpr size_t get_byte_count(size_t bits) noexcept {
            return 1 + (bits - 1) / BITS_PER_BLOCK;
        }

        static constexpr size_t block_index(size_t bit) noexcept {
            return bit / BITS_PER_BLOCK;
        }

        static constexpr size_t bit_index(size_t bit) noexcept {
            return bit % BITS_PER_BLOCK;
        }

    public:
        bitmap() noexcept : m_bits(0), m_blocks(0), m_ptr(nullptr) {
        }

        bitmap(block_type *addr, size_t bits) noexcept : m_bits(bits), m_blocks(get_byte_count(bits)), m_ptr(addr) {
        }

        bitmap(uintptr_t addr, size_t bits) noexcept : bitmap(reinterpret_cast<block_type *>(addr), bits) {
        }

        constexpr size_t bits() const noexcept {
            return m_bits;
        }

        constexpr size_t size() const noexcept {
            return m_blocks;
        }

        constexpr block_type *data() {
            return m_ptr;
        }

        constexpr const block_type *data() const {
            return m_ptr;
        }
        
        void clear() noexcept {
            for(size_t i = 0; i < m_blocks; ++i) {
                m_ptr[i] = block_type(0);
            }
        }
        
        void set() noexcept {
            for(size_t i = 0; i < m_blocks; ++i) {
                m_ptr[i] = std::numeric_limits<block_type>::max();
            }
        }

        void clear(size_t bit) noexcept {
            m_ptr[block_index(bit)] &= ~(1 << bit_index(bit));
        }
        
        void set(size_t bit) noexcept {
            m_ptr[block_index(bit)] |= (1 << bit_index(bit));
        }

        bool test_and_set(size_t bit) noexcept {
            bool value = get(bit);
            set(bit);
            return value;
        }
        
        bool test_and_clear(size_t bit) noexcept {
            bool value = get(bit);
            clear(bit);
            return value;
        }

        bool get(size_t bit) const noexcept {
            return m_ptr[block_index(bit)] & (1 << bit_index(bit));
        }

        size_t get_first_clear() const noexcept {
            for(size_t i = 0; i < m_blocks; ++i) {
                if(m_ptr[i] != std::numeric_limits<block_type>::max()) {
                    for(size_t j = 0; j < BITS_PER_BLOCK - 1; ++j) {
                        if(!(m_ptr[i] & (1 << j))) {
                            return i * BITS_PER_BLOCK + j;
                        }
                    }
                    return (i + 1) * BITS_PER_BLOCK - 1;
                }
            }
            return npos;
        }
        
        size_t get_first_set() const noexcept {
            for(size_t i = 0; i < m_blocks; ++i) {
                if(m_ptr[i] != block_type(0)) {
                    for(size_t j = 0; j < BITS_PER_BLOCK - 1; ++j) {
                        if((m_ptr[i] & (1 << j))) {
                            return i * BITS_PER_BLOCK + j;
                        }
                    }
                    return (i + 1) * BITS_PER_BLOCK - 1;
                }
            }
            return npos;
        }
    };

} // namespace util

#endif //DANAOS_KERNEL_HAL_UTIL_BITMAP_H_
