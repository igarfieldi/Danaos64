#ifndef DANAOS_ARCH_X86_64_HAL_INTERRUPTS_ISR_COUNT_H_
#define DANAOS_ARCH_X86_64_HAL_INTERRUPTS_ISR_COUNT_H_

#include <stdint.h>
#include <stddef.h>

namespace hal {

	constexpr size_t ISR_COUNT = 256;

	struct isr_registers {
		uint64_t rsp;
		uint64_t rax;
		uint64_t rcx;
		uint64_t rdx;
		uint64_t r8;
		uint64_t r9;
		uint64_t r10;
		uint64_t r11;
		uint64_t rsi;
		uint64_t rdi;
	} __attribute__((packed));

	struct isr_frame_equal_priv {
		isr_registers registers;

		uint64_t int_num;
		uint64_t error_code;
		uint64_t ret_rip;
		uint64_t cs;
		uint64_t rflags;
		uint64_t ret_rsp;
		uint64_t ret_ss;
	} __attribute__((packed));

	struct isr_frame_higher_priv {
		isr_registers registers;
		
		uint64_t int_num;
		uint64_t error_code;
		uint64_t ret_eip;
		uint64_t cs;
		uint64_t eflags;
		uint64_t ret_esp;
		uint64_t ret_ss;
	} __attribute__((packed));
};

#endif //DANAOS_ARCH_X86_64_HAL_INTERRUPTS_ISR_COUNT_H_