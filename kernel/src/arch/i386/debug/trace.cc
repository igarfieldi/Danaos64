#include "trace.h"
#include "main/kernel.h"

namespace debug {
	void backtrace() {
		backtrace(~size_t{0});
	}

	void backtrace(size_t maxFrames) {
		uintptr_t *ebp = nullptr;
		
		asm volatile(
			"movl %%ebp, %0"
			: "=r"(ebp)
			:
			:
		);
		kernel::m_console.print("Stacktrace:\n");
		
		for(size_t frame = 0; frame < maxFrames; ++frame) {
			uintptr_t *eip = &ebp[1];
			if(*eip == 0) {
				return;
			}
			
			ebp = reinterpret_cast<uintptr_t*>(ebp[0]);
			const char *name = kernel::m_elf_lookup.lookup(*eip);
			if((name == nullptr) || (*name == '\0')) {
				return ;
			}
			kernel::m_console.print("  []\t-- {}\n", *eip, name);
		}
	}

} // namespace debug
