#include "console.h"
#include "main/kernel.h"
#include "libk/math.h"

console::console() : screenX(0), screenY(0), base(radix_base::DEC) {
}

void console::put(char c) {
    switch (c) {
        case '\t':
            screenX = ((screenX / 8) + 1) * 8;
            break;
        case '\n':
            screenX = 0;
            screenY++;
            break;
        default:
            kernel::m_cga.print(c, screenX++, screenY);
    }

    while (screenX >= 80) {
        screenX -= 80;
        screenY++;
    }
    while (screenY >= 25) {
        kernel::m_cga.scroll_down();
        screenY--;
    }
}

void console::set_base(radix_base base) {
    this->base = base;
}

void console::print(const char *msg) {
    while (*msg != '\0') {
        put(*(msg++));
    }
}

void console::print(bool val) {
    if (val) {
        print("true");
    } else {
        print("false");
    }
}

void console::print(char val) {
    put(val);
}

void console::print(unsigned char val) {
    print(static_cast<unsigned long>(val));
}

void console::print(short val) {
    print(static_cast<long>(val));
}

void console::print(unsigned short val) {
    print(static_cast<unsigned long>(val));
}

void console::print(int val) {
    print(static_cast<long>(val));
}

void console::print(unsigned int val) {
    print(static_cast<unsigned long>(val));
}

void console::print(long val) {
    if (val < 0) {
        put('-');
        val *= -1;
    }
    print(static_cast<unsigned long>(val));
}

void console::print(unsigned long val) {
    unsigned long currFig, currDigit;

    switch (base) {
        case radix_base::BIN:
            print("0b");
            break;
        case radix_base::HEX:
            print("0x");
            break;
        default:
            break;
    }

    for (long i = math::logl(static_cast<uint8_t>(base), val); i >= 0; i--) {
        currFig   = math::powl(static_cast<uint8_t>(base), i);
        currDigit = val / currFig;

        // If currDigit > 9, continue with capital letters (only for hex base)
        put((char) (currDigit + 48 + (currDigit > 9 ? 7 : 0)));
        val -= currDigit * currFig;
    }
}

void console::print(void *ptr) {
    radix_base oldBase = base;
    set_base(radix_base::HEX);
    print(reinterpret_cast<uintptr_t>(ptr));
    set_base(oldBase);
}