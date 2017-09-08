#ifndef DANAOS_MAIN_CONSOLE_H_
#define DANAOS_MAIN_CONSOLE_H_

#include <stdint.h>

class console {
public:
    enum class radix_base : uint8_t { BIN = 2, DEC = 10, HEX = 16 };

private:
    void put(char c);

    void print(bool val);
    void print(char val);
    void print(unsigned char val);
    void print(short val);
    void print(unsigned short val);
    void print(int val);
    void print(unsigned int val);
    void print(long val);
    void print(unsigned long val);
    void print(void *ptr);

    uint8_t screenX, screenY;
    radix_base base;

public:
    console();
    console(const console &) = delete;

    void set_base(radix_base base);
    void print(const char *msg);

    template <class T, class... Args>
    void print(const char *msg, T val, Args... args) {
        while (*msg != '\0') {
            if (*msg == '{') {
                if (*(msg + 1) == '}') {
                    print(val);
                    print(msg + 2, args...);
                    return;
                }
            } else if (*msg == '[') {
                if (*(msg + 1) == ']') {
                    radix_base temp = base;
                    base = radix_base::HEX;
                    print(val);
                    base = temp;
                    print(msg + 2, args...);
                    return;
                }
            }

            put(*(msg++));
        }

        put('\n');
    }
};

#endif //DANAOS_MAIN_CONSOLE_H_