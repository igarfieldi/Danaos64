#include "trace.h"
#include "main/kernel.h"

namespace debug {
	void backtrace() {
		backtrace(~size_t{0});
	}

	void backtrace(size_t maxFrames) {
		uintptr_t *rbp = nullptr;
		
		asm volatile(
			"movq %%rbp, %0"
			: "=r"(rbp)
			:
			:
		);
		kernel::m_console.print("Stacktrace:\n");

		for(size_t frame = 0; frame < maxFrames; ++frame) {
			uintptr_t *rip = &rbp[1];
			if(*rip == 0) {
				return;
			}
			
			rbp = reinterpret_cast<uintptr_t*>(rbp[0]);
			const char *name = kernel::m_elf_lookup.lookup(*rip);
			if((name == nullptr) || (*name == '\0')) {
				return ;
			}
			kernel::m_console.print("  []\t-- {}\n", *rip, name);
		}
	}

} // namespace debug
