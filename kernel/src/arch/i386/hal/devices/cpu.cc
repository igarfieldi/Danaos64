#include "hal/devices/cpu.h"
#include "cpu_detail.h"

namespace hal {
	namespace cpu {

		void halt() noexcept {
			asm volatile("hlt");
		}

		void enable_int() noexcept {
			asm volatile("sti" :::"cc");
		}

		void disable_int() noexcept {
			asm volatile("cli" :::"cc");
		}

		int_state get_enable_int() noexcept {
			int_state state{};

			asm volatile("pushf\n\t"
						"sti\n\t"
						"movw (%%esp), %0\n\t"
						"add $4, %%esp"
						: "=r"(state.flags)
						:
						:"cc");

			return state;
		}

		int_state get_disable_int() noexcept {
			int_state state{};

			asm volatile("pushf\n\t"
						"cli\n\t"
						"movw (%%esp), %0\n\t"
						"add $4, %%esp"
						: "=r"(state.flags)
						:
						:"cc");

			return state;
		}

		void restore_int(const int_state &state) noexcept {
			if(state.flags & int_state::INT_MASK) {
				enable_int();
			} else {
				disable_int();
			}
		}

	} // namespace cpu
} // namespace hal