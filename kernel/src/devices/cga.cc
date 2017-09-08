#include "cga.h"
#include "libk/math.h"

namespace devices {

    cga::cga(unsigned int dim_x, unsigned int dim_y, uintptr_t loc) : m_dim_x(dim_x),
            m_dim_y(dim_y), m_screen(reinterpret_cast<volatile char*>(loc)) {
    }

    void cga::print(char c, unsigned int x, unsigned int y) {
        this->print(c, DEFAULT_ATTRIBUTE, x, y);
    }
    void cga::print(char c, char attr, unsigned int x, unsigned int y) {
        if(x < m_dim_x && y < m_dim_y) {
            m_screen[2*(x + y*m_dim_x)] = c;
            m_screen[2*(x + y*m_dim_x) + 1] = attr;
        }
    }

    void cga::scroll_down(unsigned int steps) {
        steps = math::min(m_dim_y, steps);
        for(unsigned int y = 0; y < m_dim_y - steps; ++y) {
            for(unsigned int x = 0; x < m_dim_x; ++x) {
                m_screen[2*(x + y*m_dim_x)] = m_screen[2*(x + (y + steps)*m_dim_x)];
                m_screen[2*(x + y*m_dim_x) + 1] = m_screen[2*(x + (y + steps)*m_dim_x) + 1];
            }
        }
        for(unsigned int y = m_dim_y - steps; y < m_dim_y; ++y) {
            for(unsigned int x = 0; x < m_dim_x; ++x) {
                m_screen[2*(x + y*m_dim_x)] = CLEAR_CHAR;
                m_screen[2*(x + y*m_dim_x) + 1] = DEFAULT_ATTRIBUTE;
            }
        }
    }
    void cga::clear() {
        for(unsigned int y = 0; y < m_dim_y; ++y) {
            for(unsigned int x = 0; x < m_dim_x; ++x) {
                m_screen[2*(x + y*m_dim_x)] = CLEAR_CHAR;
                m_screen[2*(x + y*m_dim_x) + 1] = DEFAULT_ATTRIBUTE;
            }
        }
    }

}