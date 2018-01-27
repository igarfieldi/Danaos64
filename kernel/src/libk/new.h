#ifndef DANAOS_LIBK_NEW_H_
#define DANAOS_LIBK_NEW_H_

#include <stddef.h>

void *operator new(size_t size, void *ptr) noexcept;

#endif //DANAOS_LIBK_NEW_H_