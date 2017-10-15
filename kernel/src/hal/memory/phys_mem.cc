#include "phys_mem.h"

namespace hal {

    phys_mem_manager::phys_mem_manager() noexcept : m_page_frame_count(0), m_phys_bitmap() {
    }

    phys_mem_manager &phys_mem_manager::instance() noexcept {
        static phys_mem_manager inst;
        return inst;
    }
    
    void phys_mem_manager::init(uintptr_t bitmap_address, size_t page_frames) noexcept {
        // Set the frame count
        m_page_frame_count = page_frames;
        // Set the bitmap at the given address and clear it
        m_phys_bitmap = util::bitmap<size_t>(bitmap_address, page_frames);
        m_phys_bitmap.clear();
        // Allocate whatever space the bitmap needs
        this->alloc_range(bitmap_address, page_frames / CHAR_BIT);
    }

    bool phys_mem_manager::is_available_frame(size_t page_frame) const noexcept {
        return !m_phys_bitmap.get(page_frame);
    }

    bool phys_mem_manager::is_available_frame(size_t page_frame, size_t frame_count) const noexcept {
        // Iterate to see if all the frames in the range are available
        for(size_t frame = page_frame; frame < page_frame + frame_count; ++frame) {
            if(m_phys_bitmap.get(frame)) {
                return false;
            }
        }
        return true;
    }

    bool phys_mem_manager::is_available_address(uintptr_t address) const noexcept {
        return !m_phys_bitmap.get(get_page_frame(address));
    }

    bool phys_mem_manager::is_available_address(uintptr_t address, size_t frame_count) const noexcept {
        // Iterate to see if all the frames in the range are available
        for(size_t frame = get_page_frame(address); frame < get_page_frame(address) + frame_count; ++frame) {
            if(m_phys_bitmap.get(frame)) {
                return false;
            }
        }
        return true;
    }

    bool phys_mem_manager::is_available_range(uintptr_t address, size_t bytes) const noexcept {
        // Iterate to see if all the frames in the range are available
        for(size_t frame = get_page_frame(address); frame < round_page_up(address + bytes); ++frame) {
            if(m_phys_bitmap.get(frame)) {
                return false;
            }
        }
        return true;
    }
        
    uintptr_t  phys_mem_manager::alloc_frame(size_t page_frame) noexcept {
        m_phys_bitmap.set(page_frame);
        return page_frame * PAGE_FRAME_SIZE;
    }

    uintptr_t  phys_mem_manager::alloc_frame(size_t page_frame, size_t frame_count) noexcept {
        // TODO: let the bitmap have its own set_range?
        for(size_t frame = page_frame; frame < page_frame + frame_count; ++frame) {
            m_phys_bitmap.set(frame);
        }
        return page_frame * PAGE_FRAME_SIZE;
    }
    
    uintptr_t  phys_mem_manager::alloc_address(uintptr_t address) noexcept {
        m_phys_bitmap.set(get_page_frame(address));
        return get_page_frame(address) * PAGE_FRAME_SIZE;
    }
    
    uintptr_t  phys_mem_manager::alloc_address(uintptr_t address, size_t frame_count) noexcept {
        // TODO: let the bitmap have its own set_range?
        for(size_t frame = get_page_frame(address); frame < round_page_up(address) + frame_count * PAGE_FRAME_SIZE; ++frame) {
            m_phys_bitmap.set(frame);
        }
        return get_page_frame(address) * PAGE_FRAME_SIZE;
    }
    
    uintptr_t  phys_mem_manager::alloc_range(uintptr_t address, size_t bytes) noexcept {
        // TODO: let the bitmap have its own set_range?
        for(size_t frame = get_page_frame(address); frame < round_page_up(address + bytes); ++frame) {
            m_phys_bitmap.set(frame);
        }
        return get_page_frame(address) * PAGE_FRAME_SIZE;
    }
    
    uintptr_t phys_mem_manager::alloc_any() noexcept {
        size_t first_clear = m_phys_bitmap.get_first_clear();
        if(first_clear != m_phys_bitmap.npos) {
            m_phys_bitmap.set(first_clear);
            return first_clear * PAGE_FRAME_SIZE;
        }
        return 0;
    }

    uintptr_t phys_mem_manager::alloc_any(size_t frame_count) noexcept {
        // TODO: use more efficient algorithm!
        size_t curr_frame = m_phys_bitmap.get_first_clear();
        while(curr_frame < m_page_frame_count) {
            size_t last_free = curr_frame;
            for(last_free = curr_frame + 1; last_free < curr_frame + frame_count; ++last_free) {
                if(!is_available_frame(last_free)) {
                    break;
                }
            }
            if(last_free >= curr_frame + frame_count) {
                for(size_t frame = curr_frame; frame < curr_frame + frame_count; ++frame) {
                    m_phys_bitmap.set(frame);
                }
                return curr_frame * PAGE_FRAME_SIZE;
            }
            curr_frame = last_free + 1;
            while((curr_frame < m_page_frame_count) && !is_available_frame(curr_frame)) {
                ++curr_frame;
            }
        }
        return 0;
    }
    
    uintptr_t phys_mem_manager::alloc_any_address(uintptr_t address_hint) noexcept {
        // TODO: use more efficient algorithm!
        for(size_t frame = get_page_frame(address_hint); frame < m_page_frame_count; ++frame) {
            if(is_available_frame(frame)) {
                m_phys_bitmap.set(frame);
                return frame;
            }
        }
        return 0;
    }
    
    uintptr_t phys_mem_manager::alloc_any_address(uintptr_t address_hint, size_t frame_count) noexcept {
        size_t curr_frame = round_page_up(address_hint);

        while(curr_frame < m_page_frame_count) {
            while((curr_frame < m_page_frame_count) && (m_phys_bitmap.get(curr_frame))) {
                ++ curr_frame;
            }

            size_t last_free = curr_frame;
            for(last_free = curr_frame + 1; last_free < curr_frame + frame_count; ++last_free) {
                if(!is_available_frame(last_free)) {
                    break;
                }
            }
            if(last_free >= curr_frame + frame_count) {
                for(size_t frame = curr_frame; frame < curr_frame + frame_count; ++frame) {
                    m_phys_bitmap.set(frame);
                }
                return curr_frame * PAGE_FRAME_SIZE;
            }
        }
        return 0;
    }
    
    uintptr_t phys_mem_manager::alloc_any_range(size_t bytes) noexcept {
        // TODO: use more efficient algorithm!
        size_t curr_frame = m_phys_bitmap.get_first_clear();
        size_t frame_count = round_page_up(bytes);
        while(curr_frame < m_page_frame_count) {
            size_t last_free = curr_frame;
            for(last_free = curr_frame + 1; last_free < curr_frame + frame_count; ++last_free) {
                if(!is_available_frame(last_free)) {
                    break;
                }
            }
            if(last_free >= curr_frame + frame_count) {
                for(size_t frame = curr_frame; frame < curr_frame + frame_count; ++frame) {
                    m_phys_bitmap.set(frame);
                }
                return curr_frame * PAGE_FRAME_SIZE;
            }
            curr_frame = last_free + 1;
            while((curr_frame < m_page_frame_count) && !is_available_frame(curr_frame)) {
                ++curr_frame;
            }
        }
        return 0;
    }
    
    uintptr_t phys_mem_manager::alloc_any_range(uintptr_t address_hint, size_t bytes) noexcept {
        // TODO: use more efficient algorithm!
        size_t curr_frame = round_page_up(address_hint);
        size_t frame_count = round_page_up(bytes);

        while(curr_frame < m_page_frame_count) {
            while((curr_frame < m_page_frame_count) && (m_phys_bitmap.get(curr_frame))) {
                ++ curr_frame;
            }

            size_t last_free = curr_frame;
            for(last_free = curr_frame + 1; last_free < curr_frame + frame_count; ++last_free) {
                if(!is_available_frame(last_free)) {
                    break;
                }
            }
            if(last_free >= curr_frame + frame_count) {
                for(size_t frame = curr_frame; frame < curr_frame + frame_count; ++frame) {
                    m_phys_bitmap.set(frame);
                }
                return curr_frame * PAGE_FRAME_SIZE;
            }
        }
        return 0;
    }

    bool phys_mem_manager::test_and_alloc_frame(size_t page_frame) noexcept {
        return !m_phys_bitmap.test_and_set(page_frame);
    }
    
    bool phys_mem_manager::test_and_alloc_frame(size_t page_frame, size_t frame_count) noexcept {
        bool val = false;
        // TODO: let the bitmap have its own set_range?
        for(size_t frame = page_frame; frame < page_frame + frame_count; ++frame) {
            val |= m_phys_bitmap.test_and_set(frame);
        }
        return !val;
    }
    
    bool phys_mem_manager::test_and_alloc_address(uintptr_t address) noexcept {
        return !m_phys_bitmap.test_and_set(get_page_frame(address));
    }
    
    bool phys_mem_manager::test_and_alloc_address(uintptr_t address, size_t frame_count) noexcept {
        bool val = false;
        // TODO: let the bitmap have its own set_range?
        for(size_t frame = get_page_frame(address); frame < round_page_up(address) + frame_count; ++frame) {
            val |= m_phys_bitmap.test_and_set(frame);
        }
        return !val;
    }
    
    bool phys_mem_manager::test_and_alloc_range(uintptr_t address, size_t bytes) noexcept {
        bool val = false;
        // TODO: let the bitmap have its own set_range?
        for(size_t frame = get_page_frame(address); frame < round_page_up(address + bytes); ++frame) {
            val |= m_phys_bitmap.test_and_set(frame);
        }
        return val;
    }
    
    void phys_mem_manager::free_frame(size_t page_frame) noexcept {
        m_phys_bitmap.clear(page_frame);
    }
    
    void phys_mem_manager::free_frame(size_t page_frame, size_t frame_count) noexcept {
        // TODO: let the bitmap have its own clear_range?
        for(size_t frame = page_frame; frame < page_frame + frame_count; ++frame) {
            m_phys_bitmap.clear(frame);
        }
    }
    
    void phys_mem_manager::free_address(uintptr_t address) noexcept {
        m_phys_bitmap.clear(get_page_frame(address));
    }
    
    void phys_mem_manager::free_address(uintptr_t address, size_t frame_count) noexcept {
        // TODO: let the bitmap have its own clear_range?
        for(size_t frame = get_page_frame(address); frame < round_page_up(address) + frame_count; ++frame) {
            m_phys_bitmap.clear(frame);
        }
    }
    
    void phys_mem_manager::free_range(uintptr_t address, size_t bytes) noexcept {
        // TODO: let the bitmap have its own clear_range?
        for(size_t frame = get_page_frame(address); frame < round_page_up(address + bytes); ++frame) {
            m_phys_bitmap.clear(frame);
        }
    }
    
    bool phys_mem_manager::test_and_free_frame(size_t page_frame) noexcept {
        return m_phys_bitmap.test_and_clear(page_frame);
    }
    
    bool phys_mem_manager::test_and_free_frame(size_t page_frame, size_t frame_count) noexcept {
        bool val = true;
        // TODO: let the bitmap have its own set_range?
        for(size_t frame = page_frame; frame < page_frame + frame_count; ++frame) {
            val &= m_phys_bitmap.test_and_clear(frame);
        }
        return val;
    }
    
    bool phys_mem_manager::test_and_free_address(uintptr_t address) noexcept {
        return !m_phys_bitmap.test_and_clear(get_page_frame(address));
    }
    
    bool phys_mem_manager::test_and_free_address(uintptr_t address, size_t frame_count) noexcept {
        bool val = true;
        // TODO: let the bitmap have its own set_range?
        for(size_t frame = get_page_frame(address); frame < round_page_up(address) + frame_count; ++frame) {
            val &= m_phys_bitmap.test_and_clear(frame);
        }
        return val;
    }
    
    bool phys_mem_manager::test_and_free_range(uintptr_t address, size_t bytes) noexcept {
        bool val = true;
        // TODO: let the bitmap have its own set_range?
        for(size_t frame = get_page_frame(address); frame < round_page_up(address + bytes); ++frame) {
            val &= m_phys_bitmap.test_and_clear(frame);
        }
        return val;
    }
    

} // namespace hal