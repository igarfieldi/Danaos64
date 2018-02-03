#ifndef DANAOS_DEVICES_CGA_H_
#define DANAOS_DEVICES_CGA_H_

#include <stdint.h>
#include <stddef.h>
#include "main/task/sync/spinlock.h"

namespace devices {

    class cga {
    private:
        static constexpr uintptr_t DEFAULT_BUFFER_LOCATION = 0xB8000;
        static constexpr size_t DEFAULT_WIDTH = 80;
        static constexpr size_t DEFAULT_HEIGHT = 25;
        static constexpr char DEFAULT_ATTRIBUTE = 15;
        static constexpr char CLEAR_CHAR = ' ';

        volatile char *m_screen;
        size_t m_width;
        size_t m_height;

        sync::spin_lock m_lock;

        cga();
        
        constexpr size_t buffer_index(size_t x, size_t y) {
        	return 2*(y*m_width + x);
        }
    
    public:
        cga(const cga&) = delete;
        cga(cga&&) = delete;
        cga& operator=(const cga&) = delete;
        cga& operator=(cga&&) = delete;

        static cga &instance() {
            static cga inst;
            return inst;
        }

        uintptr_t buffer_address() const {
            return reinterpret_cast<uintptr_t>(m_screen);
        }

        size_t width() const {
            return m_width;
        }

        size_t height() const {
            return m_height;
        }

        void init(uintptr_t framebuffer_addr, size_t dim_x, size_t dim_y);
        void print(char c, char attr, size_t x, size_t y);
        void scroll_down(size_t steps = 1, char clear_char = CLEAR_CHAR, char clear_attribute = DEFAULT_ATTRIBUTE);
        void clear(char clear_char = CLEAR_CHAR, char clear_attribute = DEFAULT_ATTRIBUTE);
    };

} // namespace devices

#endif //DANAOS_DEVICES_CGA_H_
