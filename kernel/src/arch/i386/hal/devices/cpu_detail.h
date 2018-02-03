#ifndef DANAOS_KERNEL_ARCH_I386_HAL_DEVICES_CPU_DETAIL_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_DEVICES_CPU_DETAIL_H_

#include <stdint.h>
#include "hal/devices/cpu.h"

namespace hal {
	namespace cpu {

		struct int_state {
			static constexpr uint16_t INT_MASK = 0x200;
			uint16_t flags;
		};

		struct cr0 {
			uint32_t pe			: 1;
			uint32_t mp			: 1;
			uint32_t em			: 1;
			uint32_t ts			: 1;
			uint32_t et			: 1;
			uint32_t ne			: 1;
			uint32_t 			: 10;
			uint32_t wp			: 1;
			uint32_t 			: 1;
			uint32_t am			: 1;
			uint32_t 			: 10;
			uint32_t nw			: 1;
			uint32_t cd			: 1;
			uint32_t pg			: 1;
		} __attribute__((packed));

		struct cr3 {
			uint32_t pcid		: 12;
			uint32_t pdbr		: 20;
		} __attribute__((packed));

		struct cr4 {
			uint32_t vme		: 1;
			uint32_t pvi		: 1;
			uint32_t tsd		: 1;
			uint32_t de			: 1;
			uint32_t pse		: 1;
			uint32_t pae		: 1;
			uint32_t mce		: 1;
			uint32_t pge		: 1;
			uint32_t pce		: 1;
			uint32_t osfxsr		: 1;
			uint32_t osxmmexcpt	: 1;
			uint32_t umip		: 1;
			uint32_t la57		: 1;
			uint32_t vmxe		: 1;
			uint32_t smxe		: 1;
			uint32_t 			: 1;
			uint32_t fsgsbase	: 1;
			uint32_t pcide		: 1;
			uint32_t osxsave	: 1;
			uint32_t 			: 1;
			uint32_t smep		: 1;
			uint32_t smap		: 1;
			uint32_t pke		: 1;
			uint32_t 			: 9;
		} __attribute__((packed));

		cr0 read_cr0() noexcept;
		uint32_t read_cr2() noexcept;
		cr3 read_cr3() noexcept;
		cr4 read_cr4() noexcept;
		void write_cr0(cr0 reg) noexcept;
		void write_cr3(cr3 reg) noexcept;
		void write_cr4(cr4 reg) noexcept;

	} // namespace cpu
} // namespace hal

#endif //DANAOS_KERNEL_ARCH_I386_HAL_DEVICES_CPU_DETAIL_H_