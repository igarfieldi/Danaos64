#include "new.h"

void *operator new(size_t size) {
	(void) size;
    return nullptr;
}

void *operator new(size_t size, void *ptr) noexcept {
    (void) size;
    return ptr;
}

void *operator new[](size_t size) {
	(void) size;
    return nullptr;
}

void operator delete(void *ptr) {
	(void) ptr;
}

void operator delete(void *ptr, size_t size) {
	(void) ptr;
    (void) size;
}

void operator delete[](void *ptr) {
	(void) ptr;
}

void operator delete[](void *ptr, size_t size) {
	(void) ptr;
    (void) size;
}