#include "cga.h"
#include "libk/math.h"
#include "hal/memory/virt_mem.h"

extern uintptr_t KERNEL_VIRT_OFFSET;

namespace devices {

    cga::cga() : m_screen(reinterpret_cast<volatile char*>(cga::DEFAULT_BUFFER_LOCATION)),
                m_width(DEFAULT_WIDTH), m_height(DEFAULT_HEIGHT), m_lock() {
    }
	
	void cga::init(uintptr_t framebuffer_addr, size_t width, size_t height) {
        // Temporary fix for paging
        m_screen = reinterpret_cast<volatile char*>(framebuffer_addr + reinterpret_cast<uintptr_t>(&KERNEL_VIRT_OFFSET));
        hal::virt_mem_manager::instance().map_range(reinterpret_cast<uintptr_t>(m_screen),
                    reinterpret_cast<uintptr_t>(m_screen) + width * height * 2,
                    framebuffer_addr, true, false, false);
		m_width = width;
		m_height = height;
	}

    void cga::print(char c, char attr, size_t x, size_t y) {
        if(x < m_width && y < m_height) {
            m_lock.aquire();
            m_screen[buffer_index(x, y)] = c;
            m_screen[buffer_index(x, y) + 1] = attr;
            m_lock.release();
        }
    }

    void cga::scroll_down(size_t steps, char clear_char, char clear_attribute) {
        steps = math::min(m_height, steps);
        m_lock.aquire();
        for(size_t y = 0; y < m_height - steps; ++y) {
            for(size_t x = 0; x < m_width; ++x) {
                m_screen[buffer_index(x, y)] = m_screen[buffer_index(x, y + steps)];
                m_screen[buffer_index(x, y) + 1] = m_screen[buffer_index(x, y + steps) + 1];
            }
        }
        for(size_t y = m_height - steps; y < m_height; ++y) {
            for(size_t x = 0; x < m_width; ++x) {
                m_screen[buffer_index(x, y)] = clear_char;
                m_screen[buffer_index(x, y) + 1] = clear_attribute;
            }
        }
        m_lock.release();
    }
    void cga::clear(char clear_char, char clear_attribute) {
        m_lock.aquire();
        for(size_t y = 0; y < m_height; ++y) {
            for(size_t x = 0; x < m_width; ++x) {
                m_screen[buffer_index(x, y)] = clear_char;
                m_screen[buffer_index(x, y) + 1] = clear_attribute;
            }
        }
        m_lock.release();
    }

}
