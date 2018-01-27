#include "new.h"

void *operator new(size_t size, void *ptr) noexcept {
	(void) size;
	return ptr;
}