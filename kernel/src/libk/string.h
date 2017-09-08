#ifndef DANAOS_LIBK_STRING_H_
#define DANAOS_LIBK_STRING_H_

#include <stddef.h>

namespace std {
    size_t strlen(const char *str);
    int strncmp(const char *str1, const char *str2, size_t count);
}

#endif // DANAOS_LIBK_STRING_H_