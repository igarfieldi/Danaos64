#ifndef DANAOS_DEVICES_CGA_H_
#define DANAOS_DEVICES_CGA_H_

#include <stdint.h>

namespace devices {

    class cga {
    private:
        static constexpr char DEFAULT_ATTRIBUTE = 15;
        static constexpr char CLEAR_CHAR = ' ';

        const unsigned int m_dim_x;
        const unsigned int m_dim_y;

        volatile char *m_screen;

    public:
        cga(unsigned int dim_x, unsigned int dim_y, uintptr_t loc);

        void print(char c, unsigned int x, unsigned int y);
        void print(char c, char attr, unsigned int x, unsigned int y);

        void scroll_down(unsigned int steps = 1);
        void clear();
    };

} // namespace devices

#endif //DANAOS_DEVICES_CGA_H_