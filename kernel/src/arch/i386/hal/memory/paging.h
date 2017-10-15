#ifndef DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_PAGING_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_PAGING_H_

namespace hal {

    struct page_directory_entry {
        uint32_t present            : 1;
        uint32_t read_write         : 1;
        uint32_t user_supervisor    : 1;
        uint32_t write_through      : 1;
        uint32_t cache_disabled     : 1;
        uint32_t accessed           : 1;
        uint32_t reserved           : 1;
        uint32_t size_4mb           : 1;
        uint32_t ignored            : 4;
        uint32_t table_address      : 20;

        page_directory_entry() : present(0), read_write(0), user_supervisor(0),
                write_through(0), cache_disabled(0), accessed(0), reserved(0),
                size_4mb(0), ignored(0), table_address(0) {
        }
        
        page_directory_entry(uint32_t raw) : present(raw & 0x1), read_write((raw >> 1) & 0x1),
                user_supervisor((raw >> 2) & 0x1), write_through((raw >> 3) & 0x1),
                cache_disabled((raw >> 4) & 0x1), accessed((raw >> 5) & 0x1), reserved(0),
                size_4mb((raw >> 7) & 0x1), ignored((raw >> 8) & 0xF), table_address((raw >> 12) & 0xFFFFF) {
        }
        
        page_directory_entry(bool present, bool read_write, bool user_supervisor,
            bool write_through, bool cache_disabled, bool size_4mb,
            uint32_t table_address) : present(present), read_write(read_write), user_supervisor(user_supervisor),
                write_through(write_through), cache_disabled(cache_disabled), accessed(0), reserved(0),
                size_4mb(size_4mb), ignored(0), table_address((table_address >> 12) & 0xFFFFF) {
        }
    } __attribute__((packed));

    struct page {
        uint32_t present            : 1;
        uint32_t read_write         : 1;
        uint32_t user_supervisor    : 1;
        uint32_t write_through      : 1;
        uint32_t cache_disabled     : 1;
        uint32_t accessed           : 1;
        uint32_t dirty              : 1;
        uint32_t reserved           : 1;
        uint32_t global             : 1;
        uint32_t ignored            : 3;
        uint32_t physical_address   : 20;
        
        page() : present(0), read_write(0), user_supervisor(0),
                write_through(0), cache_disabled(0), accessed(0), dirty(0),
                reserved(0), global(0), ignored(0), physical_address(0) {
        }
        
        page(uint32_t raw) : present(raw & 0x1), read_write((raw >> 1) & 0x1),
                user_supervisor((raw >> 2) & 0x1), write_through((raw >> 3) & 0x1),
                cache_disabled((raw >> 4) & 0x1), accessed((raw >> 5) & 0x1), dirty((raw >> 6) & 0x1),
                reserved(0), global((raw >> 8) & 0x1), ignored((raw >> 9) & 0xF),
                physical_address((raw >> 12) & 0xFFFFF) {
        }
        
        page(bool present, bool read_write, bool user_supervisor,
            bool write_through, bool cache_disabled, bool global,
            uint32_t physical_address) : present(present), read_write(read_write), user_supervisor(user_supervisor),
                write_through(write_through), cache_disabled(cache_disabled), accessed(0), dirty(0),
                reserved(0), global(global), ignored(0), physical_address((physical_address >> 12) & 0xFFFFF) {
        }
    } __attribute__((packed));

    struct page_table {
    public:
        static constexpr size_t ENTRIES = 1024;
    
    private:
        page m_pages[ENTRIES];

    public:
        page &operator[](size_t index) {
            return m_pages[index];
        }

        const page &operator[](size_t index) const {
            return m_pages[index];
        }
    } __attribute__((packed));

} // namespace hal


#endif //DANAOS_KERNEL_ARCH_I386_HAL_MEMORY_PAGING_H_