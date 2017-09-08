#include "string.h"

namespace std {

    size_t strlen(const char *str) {
        size_t len = 0;
        while(*(str++) != '\0') {
            ++len;
        }
        return len;
    }

    int strncmp(const char *str1, const char *str2, size_t count) {
        while (count-- > 0 && (*str1) != 0 && (*str2) != 0) {
            if ((*str1) < (*str2)) {
                return -1;
            } else if ((*str1) > (*str2)) {
                return 1;
            }
            str1++;
            str2++;
        }

        if (count == 0 || (*str1) == (*str2)) {
            return 0;
        } else if ((*str1) == 0) {
            return -1;
        } else {
            return 1;
        }
    }

} // namespace std