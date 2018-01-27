#ifndef DANAOS_MAIN_DEBUG_H_
#define DANAOS_MAIN_DEBUG_H_

#include <stdint.h>
#include <stddef.h>

namespace debug {

	void backtrace();
	void backtrace(size_t maxFrames);

} // namespace debug

#endif //DANAOS_MAIN_DEBUG_H_
