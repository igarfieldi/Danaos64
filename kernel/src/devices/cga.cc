#include "cga.h"
#include "libk/math.h"

namespace devices {

    cga::cga() : m_screen(reinterpret_cast<volatile char*>(cga::DEFAULT_BUFFER_LOCATION)),
                m_width(DEFAULT_WIDTH), m_height(DEFAULT_HEIGHT) {
    }
	
	void cga::init(uintptr_t framebuffer_addr, size_t width, size_t height) {
		m_screen = reinterpret_cast<volatile char*>(framebuffer_addr);
		m_width = width;
		m_height = height;
	}
	
    void cga::print(char c, char attr, size_t x, size_t y) {
        if(x < m_width && y < m_height) {
            m_screen[buffer_index(x, y)] = c;
            m_screen[buffer_index(x, y) + 1] = attr;
        }
    }

    void cga::scroll_down(size_t steps, char clear_char, char clear_attribute) {
        steps = math::min(m_height, steps);
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
    }
    void cga::clear(char clear_char, char clear_attribute) {
        for(size_t y = 0; y < m_height; ++y) {
            for(size_t x = 0; x < m_width; ++x) {
                m_screen[buffer_index(x, y)] = clear_char;
                m_screen[buffer_index(x, y) + 1] = clear_attribute;
            }
        }
    }

}
