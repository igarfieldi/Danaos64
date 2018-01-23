#ifndef DANAOS_KERNEL_ARCH_X86_64_HAL_MEMORY_PAGING_H_
#define DANAOS_KERNEL_ARCH_X86_64_HAL_MEMORY_PAGING_H_

#include <stdint.h>
#include <stddef.h>
#include "hal/util/bitmap.h"

namespace hal {

    class page_dir_ptr_entry {
    private:
        uint64_t m_raw;

    public:
        page_dir_ptr_entry() = default;
        page_dir_ptr_entry(const page_dir_ptr_entry &) = default;
        page_dir_ptr_entry(page_dir_ptr_entry &&) = default;
        page_dir_ptr_entry &operator=(const page_dir_ptr_entry &) = default;
        page_dir_ptr_entry &operator=(page_dir_ptr_entry &&) = default;

        page_dir_ptr_entry(uint64_t raw) : m_raw(raw) {}
        page_dir_ptr_entry(uintptr_t phys_addr, bool present, bool rw, bool user, bool wt, bool cd,
                        bool attr_table, bool global, bool no_exec) :
                m_raw(present | (rw << 1) | (user << 2) | (wt << 3) | (cd << 4) | (attr_table << 12)
                        | (global << 8) | (phys_addr & 0xFFFFFC0000000)
                        | (static_cast<uint64_t>(no_exec) << 63)) {}
        page_dir_ptr_entry(uintptr_t dir_addr, bool present, bool rw, bool user, bool wt, bool cd, bool no_exec) :
                m_raw(present | (rw << 1) | (user << 2) | (wt << 3) | (cd << 4)
                        | (dir_addr & 0xFFFFFFFFFF000) | (static_cast<uint64_t>(no_exec) << 63)) {}

        constexpr bool is_present() const           { return util::get_bit(m_raw, 0); }
        constexpr bool is_read_write() const        { return util::get_bit(m_raw, 1); }
        constexpr bool is_user() const              { return util::get_bit(m_raw, 2); }
        constexpr bool is_write_through() const     { return util::get_bit(m_raw, 3); }
        constexpr bool is_cache_disabled() const    { return util::get_bit(m_raw, 4); }
        constexpr bool is_accessed() const          { return util::get_bit(m_raw, 5); }
        constexpr bool is_dirty() const             { return util::get_bit(m_raw, 6); }         // Only for 1GB
        constexpr bool is_global() const            { return util::get_bit(m_raw, 8); }         // Only for 1GB
        constexpr bool get_attr_table() const       { return util::get_bit(m_raw, 12); }        // Only for 1GB
        constexpr uintptr_t get_phys_addr() const   { return util::get_bits(m_raw, 30, 22); }   // Only for 1GB
        constexpr uintptr_t get_dir_addr() const    { return util::get_bits(m_raw, 12, 40); }   // Only for 4KB and 2MB
        constexpr bool is_no_exec() const           { return util::get_bit(m_raw, 63); }
        constexpr bool is_size_2mb() const          { return util::get_bit(m_raw, 7); }
        
        void set_present(bool val)                  { m_raw = util::set_bit(m_raw, 0, val); }
        void set_read_write(bool val)               { m_raw = util::set_bit(m_raw, 1, val); }
        void set_user(bool val)                     { m_raw = util::set_bit(m_raw, 2, val); }
        void set_write_through(bool val)            { m_raw = util::set_bit(m_raw, 3, val); }
        void set_cache_disabled(bool val)           { m_raw = util::set_bit(m_raw, 4, val); }
        void set_accessed(bool val)                 { m_raw = util::set_bit(m_raw, 5, val); }
        void set_dirty(bool val)                    { m_raw = util::set_bit(m_raw, 6, val); }           // Only for 1GB
        void set_global(bool val)                   { m_raw = util::set_bit(m_raw, 8, val); }           // Only for 1GB
        void set_attr_table(bool val)               { m_raw = util::set_bit(m_raw, 12, val); }          // Only for 1GB
        void set_phys_addr(uintptr_t addr)          { m_raw = util::set_bits(m_raw, 30, 22, addr); }    // Only for 1GB
        void set_dir_addr(uintptr_t addr)           { m_raw = util::set_bits(m_raw, 12, 40, addr); }    // Only for 4KB and 2MB
        void set_no_exec(bool val)                  { m_raw = util::set_bit(m_raw, 63, val); }
        
        void set_size_4kb_or_2mb() {
            m_raw = util::set_bit(m_raw, 7, 0);
        }

        void set_size_1gb() {
            // Clear all bits that become relevant
            m_raw = util::set_bit(m_raw, 7, 1);
            m_raw = util::set_bit(m_raw, 13, 17);
            this->set_accessed(false);
            this->set_dirty(false);
            this->set_global(false);
            this->set_attr_table(false);
        }
    } __attribute__((packed));

    class page_dir_entry {
    private:
        uint64_t m_raw;

    public:
        page_dir_entry() = default;
        page_dir_entry(const page_dir_entry &) = default;
        page_dir_entry(page_dir_entry &&) = default;
        page_dir_entry &operator=(const page_dir_entry &) = default;
        page_dir_entry &operator=(page_dir_entry &&) = default;

        page_dir_entry(uint64_t raw) : m_raw(raw) {}
        page_dir_entry(uintptr_t phys_addr, bool present, bool rw, bool user, bool wt, bool cd,
                        bool attr_table, bool global, bool no_exec) :
                m_raw(present | (rw << 1) | (user << 2) | (wt << 3) | (cd << 4) | (attr_table << 12)
                        | (global << 8) | (phys_addr & 0xFFFFFFFE00000)
                        | (static_cast<uint64_t>(no_exec) << 63)) {}
        page_dir_entry(uintptr_t table_addr, bool present, bool rw, bool user, bool wt, bool cd, bool no_exec) :
                m_raw(present | (rw << 1) | (user << 2) | (wt << 3) | (cd << 4)
                        | (table_addr & 0xFFFFFFFFFF000) | (static_cast<uint64_t>(no_exec) << 63)) {}

        constexpr bool is_present() const           { return util::get_bit(m_raw, 0); }
        constexpr bool is_read_write() const        { return util::get_bit(m_raw, 1); }
        constexpr bool is_user() const              { return util::get_bit(m_raw, 2); }
        constexpr bool is_write_through() const     { return util::get_bit(m_raw, 3); }
        constexpr bool is_cache_disabled() const    { return util::get_bit(m_raw, 4); }
        constexpr bool is_accessed() const          { return util::get_bit(m_raw, 5); }
        constexpr bool is_dirty() const             { return util::get_bit(m_raw, 6); }         // Only for 2MB
        constexpr bool is_global() const            { return util::get_bit(m_raw, 8); }         // Only for 2MB
        constexpr bool get_attr_table() const       { return util::get_bit(m_raw, 12); }        // Only for 2MB
        constexpr uintptr_t get_phys_addr() const   { return util::get_bits(m_raw, 21, 31); }   // Only for 2MB
        constexpr uintptr_t get_table_addr() const  { return util::get_bits(m_raw, 12, 40); }   // Only for 4KB
        constexpr bool is_no_exec() const           { return util::get_bit(m_raw, 63); }
        constexpr bool is_size_2mb() const          { return util::get_bit(m_raw, 7); }
        
        void set_present(bool val)                  { m_raw = util::set_bit(m_raw, 0, val); }
        void set_read_write(bool val)               { m_raw = util::set_bit(m_raw, 1, val); }
        void set_user(bool val)                     { m_raw = util::set_bit(m_raw, 2, val); }
        void set_write_through(bool val)            { m_raw = util::set_bit(m_raw, 3, val); }
        void set_cache_disabled(bool val)           { m_raw = util::set_bit(m_raw, 4, val); }
        void set_accessed(bool val)                 { m_raw = util::set_bit(m_raw, 5, val); }
        void set_dirty(bool val)                    { m_raw = util::set_bit(m_raw, 6, val); }           // Only for 2MB
        void set_global(bool val)                   { m_raw = util::set_bit(m_raw, 8, val); }           // Only for 2MB
        void set_attr_table(bool val)               { m_raw = util::set_bit(m_raw, 12, val); }          // Only for 2MB
        void set_phys_addr(uintptr_t addr)          { m_raw = util::set_bits(m_raw, 21, 31, addr); }    // Only for 2MB
        void set_table_addr(uintptr_t addr)         { m_raw = util::set_bits(m_raw, 12, 40, addr); }    // Only for 4KB
        void set_no_exec(bool val)                  { m_raw = util::set_bit(m_raw, 63, val); }
        
        void set_size_4kb() {
            m_raw = util::set_bit(m_raw, 7, 0);
        }

        void set_size_2mb() {
            // Clear all bits that become relevant
            m_raw = util::set_bit(m_raw, 7, 1);
            m_raw = util::set_bit(m_raw, 13, 8);
            this->set_accessed(false);
            this->set_dirty(false);
            this->set_global(false);
            this->set_attr_table(false);
        }
    } __attribute__((packed));

    class page_table_entry {
    private:
        uint64_t m_raw;

    public:
        page_table_entry() = default;
        page_table_entry(const page_table_entry &) = default;
        page_table_entry(page_table_entry &&) = default;
        page_table_entry &operator=(const page_table_entry &) = default;
        page_table_entry &operator=(page_table_entry &&) = default;

        page_table_entry(uint64_t raw) : m_raw(raw) {}
        page_table_entry(uintptr_t phys_addr, bool present, bool rw, bool user, bool wt, bool cd,
                        bool attr_table, bool global, bool no_exec) :
                m_raw(present | (rw << 1) | (user << 2) | (wt << 3) | (cd << 4) | (attr_table << 7)
                        | (global << 8) | (phys_addr & 0xFFFFFFFFFF000)
                        | (static_cast<uint64_t>(no_exec) << 63)) {}

        constexpr bool is_present() const           { return util::get_bit(m_raw, 0); }
        constexpr bool is_read_write() const        { return util::get_bit(m_raw, 1); }
        constexpr bool is_user() const              { return util::get_bit(m_raw, 2); }
        constexpr bool is_write_through() const     { return util::get_bit(m_raw, 3); }
        constexpr bool is_cache_disabled() const    { return util::get_bit(m_raw, 4); }
        constexpr bool is_accessed() const          { return util::get_bit(m_raw, 5); }
        constexpr bool is_dirty() const             { return util::get_bit(m_raw, 6); }
        constexpr bool get_attr_table() const       { return util::get_bit(m_raw, 7); }
        constexpr bool is_global() const            { return util::get_bit(m_raw, 8); }
        constexpr uintptr_t get_phys_addr() const   { return util::get_bits(m_raw, 12, 40); }
        constexpr bool is_no_exec() const           { return util::get_bit(m_raw, 63); }
        
        void set_present(bool val)                  { m_raw = util::set_bit(m_raw, 0, val); }
        void set_read_write(bool val)               { m_raw = util::set_bit(m_raw, 1, val); }
        void set_user(bool val)                     { m_raw = util::set_bit(m_raw, 2, val); }
        void set_write_through(bool val)            { m_raw = util::set_bit(m_raw, 3, val); }
        void set_cache_disabled(bool val)           { m_raw = util::set_bit(m_raw, 4, val); }
        void set_accessed(bool val)                 { m_raw = util::set_bit(m_raw, 5, val); }
        void set_dirty(bool val)                    { m_raw = util::set_bit(m_raw, 6, val); }
        void set_attr_table(bool val)               { m_raw = util::set_bit(m_raw, 7, val); }
        void set_global(bool val)                   { m_raw = util::set_bit(m_raw, 8, val); }
        void set_phys_addr(uintptr_t addr)          { m_raw = util::set_bits(m_raw, 12, 40, addr); }
        void set_no_exec(bool val)                  { m_raw = util::set_bit(m_raw, 63, val); }
    } __attribute__((packed));

    template < class T, size_t E >
    struct table {
    public:
        using type = T;
        static constexpr size_t ENTRIES = E;
    
    private:
        T m_entries[ENTRIES];

    public:
        T &operator[](size_t index) {
            return m_entries[index];
        }

        const T &operator[](size_t index) const {
            return m_entries[index];
        }
    } __attribute__((packed));

} // namespace hal


#endif //DANAOS_KERNEL_ARCH_X86_64_HAL_MEMORY_PAGING_H_