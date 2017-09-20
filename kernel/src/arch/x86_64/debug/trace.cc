#include "trace.h"
#include "main/kernel.h"

namespace debug {

	void backtrace(size_t maxFrames) {
		uintptr_t *rbp = nullptr;
		
		asm volatile(
			"movq %%rbp, %0"
			: "=r"(rbp)
			:
			:
		);
		
		for(size_t frame = 0; frame < maxFrames; ++frame) {
			uintptr_t *rip = &rbp[1];
			if(*rip == 0) {
				return;
			}
			
			rbp = reinterpret_cast<uintptr_t*>(rbp[0]);
			const char *name = kernel::m_elf_lookup.lookup(*rip);
			kernel::m_console.print("Method: {} (Address [])\n",
									name, *rip);
		}
	}

} // namespace debug
