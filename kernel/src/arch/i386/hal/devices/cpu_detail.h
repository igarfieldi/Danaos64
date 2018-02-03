#ifndef DANAOS_KERNEL_ARCH_I386_HAL_DEVICES_CPU_DETAIL_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_DEVICES_CPU_DETAIL_H_

#include <stdint.h>

namespace hal {
	namespace cpu {

		struct int_state {
			static constexpr uint16_t INT_MASK = 0x200;
			uint16_t flags;
		};

	} // namespace cpu
} // namespace hal

#endif //DANAOS_KERNEL_ARCH_I386_HAL_DEVICES_CPU_DETAIL_H_