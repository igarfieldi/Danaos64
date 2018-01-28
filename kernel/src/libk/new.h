#ifndef DANAOS_LIBK_NEW_H_
#define DANAOS_LIBK_NEW_H_

#include <stddef.h>

void *operator new(size_t size);
void *operator new(size_t size, void *ptr) noexcept;
void *operator new[](size_t size);

void operator delete(void *ptr);
void operator delete(void *ptr, size_t size);
void operator delete[](void *ptr);
void operator delete[](void *ptr, size_t size);

#endif //DANAOS_LIBK_NEW_H_