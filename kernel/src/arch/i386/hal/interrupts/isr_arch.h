#ifndef DANAOS_ARCH_I386_HAL_INTERRUPTS_ISR_ARCH_H_
#define DANAOS_ARCH_I386_HAL_INTERRUPTS_ISR_ARCH_H_

#include <stdint.h>
#include <stddef.h>

namespace hal {

	constexpr size_t ISR_COUNT = 256;

	struct isr_registers {
		uint32_t esp;
		uint32_t eax;
		uint32_t ecx;
		uint32_t edx;
	} __attribute__((packed));

	struct isr_frame_equal_priv {
		isr_registers registers;

		uint32_t int_num;
		uint32_t error_code;
		uint32_t ret_eip;
		uint32_t cs;
		uint32_t eflags;
	} __attribute__((packed));

	struct isr_frame_higher_priv {
		isr_registers registers;

		uint32_t int_num;
		uint32_t error_code;
		uint32_t ret_eip;
		uint32_t cs;
		uint32_t eflags;
		uint32_t ret_esp;
		uint32_t ret_ss;
	} __attribute__((packed));

};

#endif //DANAOS_ARCH_I386_HAL_INTERRUPTS_ISR_ARCH_H_