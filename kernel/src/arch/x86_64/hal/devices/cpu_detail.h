#ifndef DANAOS_KERNEL_ARCH_X86_64_HAL_DEVICES_CPU_DETAIL_H_
#define DANAOS_KERNEL_ARCH_X86_64_HAL_DEVICES_CPU_DETAIL_H_

#include <stdint.h>
#include "hal/devices/cpu.h"

namespace hal {
	namespace cpu {

		struct int_state {
			static constexpr uint16_t INT_MASK = 0x200;
			uint16_t flags;
		};

		struct cr0 {
			uint64_t pe			: 1;
			uint64_t mp			: 1;
			uint64_t em			: 1;
			uint64_t ts			: 1;
			uint64_t et			: 1;
			uint64_t ne			: 1;
			uint64_t 			: 10;
			uint64_t wp			: 1;
			uint64_t 			: 1;
			uint64_t am			: 1;
			uint64_t 			: 10;
			uint64_t nw			: 1;
			uint64_t cd			: 1;
			uint64_t pg			: 1;
			uint64_t			: 32;
		} __attribute__((packed));

		struct cr3 {
			uint64_t pcid		: 12;
			uint64_t pdbr		: 52;
		} __attribute__((packed));

		struct cr4 {
			uint64_t vme		: 1;
			uint64_t pvi		: 1;
			uint64_t tsd		: 1;
			uint64_t de			: 1;
			uint64_t pse		: 1;
			uint64_t pae		: 1;
			uint64_t mce		: 1;
			uint64_t pge		: 1;
			uint64_t pce		: 1;
			uint64_t osfxsr		: 1;
			uint64_t osxmmexcpt	: 1;
			uint64_t umip		: 1;
			uint64_t la57		: 1;
			uint64_t vmxe		: 1;
			uint64_t smxe		: 1;
			uint64_t 			: 1;
			uint64_t fsgsbase	: 1;
			uint64_t pcide		: 1;
			uint64_t osxsave	: 1;
			uint64_t 			: 1;
			uint64_t smep		: 1;
			uint64_t smap		: 1;
			uint64_t pke		: 1;
			uint64_t 			: 41;
		} __attribute__((packed));

		cr0 read_cr0() noexcept;
		uint64_t read_cr2() noexcept;
		cr3 read_cr3() noexcept;
		cr4 read_cr4() noexcept;
		void write_cr0(cr0 reg) noexcept;
		void write_cr3(cr3 reg) noexcept;
		void write_cr4(cr4 reg) noexcept;

	} // namespace cpu
} // namespace hal

#endif //DANAOS_KERNEL_ARCH_X86_64_HAL_DEVICES_CPU_DETAIL_H_