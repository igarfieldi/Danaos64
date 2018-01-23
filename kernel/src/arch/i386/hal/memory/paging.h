#ifndef DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_PAGING_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_PAGING_H_

#include <stdint.h>
#include <stddef.h>
#include "hal/util/bitmap.h"

namespace hal {

    class page_dir_entry {
    private:
        uint32_t m_raw;

    public:
        page_dir_entry() = default;
        page_dir_entry(const page_dir_entry &) = default;
        page_dir_entry(page_dir_entry &&) = default;
        page_dir_entry &operator=(const page_dir_entry &) = default;
        page_dir_entry &operator=(page_dir_entry &&) = default;

        page_dir_entry(uint32_t raw) : m_raw(raw) {}
        page_dir_entry(uintptr_t table_addr, bool present, bool rw, bool user, bool wt, bool cd) :
                m_raw(present | (rw << 1) | (user << 2) | (wt << 3) | (cd << 4) | (table_addr & 0xFFFFF000)) {}
        page_dir_entry(uintptr_t phys_addr, bool present, bool rw, bool user, bool wt, bool cd, bool global) :
                m_raw(present | (rw << 1) | (user << 2) | (wt << 3) | (cd << 4) | (1 << 7)
                            | (global << 8) | (phys_addr & 0xFFFFF000)) {}

        constexpr bool is_present() const           { return util::get_bit(m_raw, 0); }
        constexpr bool is_read_write() const        { return util::get_bit(m_raw, 1); }
        constexpr bool is_user() const              { return util::get_bit(m_raw, 2); }
        constexpr bool is_write_through() const     { return util::get_bit(m_raw, 3); }
        constexpr bool is_cache_disabled() const    { return util::get_bit(m_raw, 4); }
        constexpr bool is_accessed() const          { return util::get_bit(m_raw, 5); }
        constexpr bool is_size_4mb() const          { return util::get_bit(m_raw, 7); }
        constexpr bool is_dirty() const             { return util::get_bit(m_raw, 6); }             // Only for 4MB pages
        constexpr bool is_global() const            { return util::get_bit(m_raw, 8); }             // Only for 4MB pages
        constexpr uintptr_t get_phys_addr() const   { return util::get_bits(m_raw, 12, 20); }       // Only for 4MB pages
        constexpr uintptr_t get_table_addr() const  { return util::get_bits(m_raw, 12, 20); }       // Only for 4KB pages
        
        void set_present(bool val)                  { m_raw = util::set_bit(m_raw, 0, val); }
        void set_read_write(bool val)               { m_raw = util::set_bit(m_raw, 1, val); }
        void set_user(bool val)                     { m_raw = util::set_bit(m_raw, 2, val); }
        void set_write_through(bool val)            { m_raw = util::set_bit(m_raw, 3, val); }
        void set_cache_disabled(bool val)           { m_raw = util::set_bit(m_raw, 4, val); }
        void set_accessed(bool val)                 { m_raw = util::set_bit(m_raw, 5, val); }
        void set_dirty(bool val)                    { m_raw = util::set_bit(m_raw, 6, val); }        // Only for 4MB pages
        void set_global(bool val)                   { m_raw = util::set_bit(m_raw, 8, val); }        // Only for 4MB pages
        void set_phys_addr(uintptr_t addr)          { m_raw = util::set_bits(m_raw, 12, 20, addr); } // Only for 4MB pages
        void set_table_addr(uintptr_t addr)         { m_raw = util::set_bits(m_raw, 12, 20, addr); } // Only for 4KB pages

        void set_size_4kb() {
            m_raw = util::set_bit(m_raw, 7, 0);
        }

        void set_size_4mb() {
            // Clear all bits that become relevant
            m_raw = util::set_bit(m_raw, 7, 1);
            this->set_accessed(false);
            this->set_dirty(false);
            this->set_global(false);
        }
    } __attribute__((packed));

    class page_table_entry {
    private:
        uint32_t m_raw;

    public:
        page_table_entry() = default;
        page_table_entry(const page_table_entry &) = default;
        page_table_entry(page_table_entry &&) = default;
        page_table_entry &operator=(const page_table_entry &) = default;
        page_table_entry &operator=(page_table_entry &&) = default;

        page_table_entry(uint32_t raw) : m_raw(raw) {}
        page_table_entry(uintptr_t phys_addr, bool present, bool rw, bool user, bool wt, bool cd, bool global) :
                m_raw(present | (rw << 1) | (user << 2) | (wt << 3) | (cd << 4) | (global << 8)
                        | (phys_addr & 0xFFFFF000)) {}

        constexpr bool is_present() const           { return util::get_bit(m_raw, 0); }
        constexpr bool is_read_write() const        { return util::get_bit(m_raw, 1); }
        constexpr bool is_user() const              { return util::get_bit(m_raw, 2); }
        constexpr bool is_write_through() const     { return util::get_bit(m_raw, 3); }
        constexpr bool is_cache_disabled() const    { return util::get_bit(m_raw, 4); }
        constexpr bool is_accessed() const          { return util::get_bit(m_raw, 5); }
        constexpr bool is_dirty() const             { return util::get_bit(m_raw, 6); }
        constexpr bool is_global() const            { return util::get_bit(m_raw, 8); }
        constexpr uintptr_t get_phys_addr() const   { return util::get_bits(m_raw, 12, 20); }
        
        void set_present(bool val)                  { m_raw = util::set_bit(m_raw, 0, val); }
        void set_read_write(bool val)               { m_raw = util::set_bit(m_raw, 1, val); }
        void set_user(bool val)                     { m_raw = util::set_bit(m_raw, 2, val); }
        void set_write_through(bool val)            { m_raw = util::set_bit(m_raw, 3, val); }
        void set_cache_disabled(bool val)           { m_raw = util::set_bit(m_raw, 4, val); }
        void set_accessed(bool val)                 { m_raw = util::set_bit(m_raw, 5, val); }
        void set_dirty(bool val)                    { m_raw = util::set_bit(m_raw, 6, val); }
        void set_global(bool val)                   { m_raw = util::set_bit(m_raw, 8, val); }
        void set_phys_addr(uintptr_t addr)          { m_raw = util::set_bits(m_raw, 12, 20, addr); }
    } __attribute__((packed));
    
    struct page_dir {
    public:
        static constexpr size_t ENTRIES = 1024;
    
    private:
         page_dir_entry m_entries[ENTRIES];

    public:
         page_dir_entry &entry(size_t dir)  noexcept {
            return m_entries[dir];
        }

         const page_dir_entry &entry(size_t dir) const  noexcept {
            return m_entries[dir];
        }
    } __attribute__((packed));

    struct page_table {
    public:
        static constexpr size_t ENTRIES = 1024;
    
    private:
         page_table_entry m_entries[page_dir::ENTRIES * ENTRIES];

    public:
         page_table_entry &entry(size_t dir, size_t table)  noexcept {
            return m_entries[table + dir * page_dir::ENTRIES];
        }

         const page_table_entry &entry(size_t dir, size_t table) const  noexcept {
            return m_entries[table + dir * page_dir::ENTRIES];
        }
    } __attribute__((packed));

} // namespace hal


#endif //DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_PAGING_H_
