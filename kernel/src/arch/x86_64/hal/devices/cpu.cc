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

		cr0 read_cr0() noexcept {
			cr0 reg;
			asm volatile("movq %%cr0, %0" : "=r"(reg) ::);
			return reg;
		}

		uint64_t read_cr2() noexcept {
			uint64_t reg;
			asm volatile("movq %%cr2, %0" : "=r"(reg) ::);
			return reg;
		}

		cr3 read_cr3() noexcept {
			cr3 reg;
			asm volatile("movq %%cr3, %0" : "=r"(reg) ::);
			return reg;
		}

		cr4 read_cr4() noexcept {
			cr4 reg;
			asm volatile("movq %%cr4, %0" : "=r"(reg) ::);
			return reg;
		}

		void write_cr0(cr0 reg) noexcept {
			asm volatile("movq %0, %%cr0" :: "r"(reg) :);
		}

		void write_cr3(cr3 reg) noexcept {
			asm volatile("movq %0, %%cr3" :: "r"(reg) :);
		}

		void write_cr4(cr4 reg) noexcept {
			asm volatile("movq %0, %%cr4" :: "r"(reg) :);
		}

	} // namespace cpu
} // namespace hal