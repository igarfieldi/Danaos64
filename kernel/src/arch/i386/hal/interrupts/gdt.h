#ifndef DANAOS_KERNEL_ARCH_I386_HAL_INTERRUPTS_GDT_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_INTERRUPTS_GDT_H_

#include <stdint.h>
#include "hal/util/bitmap.h"

namespace hal {

	class glob_desc_table {
		enum class dpl_level {
			RING_0 = 0,
			RING_1 = 1,
			RING_2 = 2,
			RING_3 = 3
		};

		class entry {
		private:
			uint64_t m_raw;

		public:
			entry() = default;
			entry(const entry &) = default;
			entry(entry &&) = default;
			entry &operator=(const entry &) = default;
			entry &operator(entry &&) = default;

			entry(uint64_t raw) noexcept : m_raw(raw) {}
			entry(uint32_t base, uint32_t limit, dpl_level priv, bool present, bool rw, bool down, bool exec, bool size32,
					bool gran4kb) noexcept : m_raw((limit & 0xFFFF) | ((base & 0xFFFFFF) << 16)
													| (rw << 41) | (down << 42) | (exec << 43)
													| (1 << 44) | (static_cast<uint32_t>(priv) << 45)
													| (present << 47) | ((limit & 0xF0000) << 48)
													| (size32 << 54) | (gran4kb << 55)
													| ((base & 0xFFFFFF) << 56) {}

			constexpr bool is_accessed() const				{ return util::get_bit(m_raw, 40); }
			constexpr bool is_read_write() const			{ return util::get_bit(m_raw, 41); }
			constexpr bool is_growing_down() const			{ return util::get_bit(m_raw, 42); }
			constexpr bool is_executable() const			{ return util::get_bit(m_raw, 43); }
			constexpr bool is_present() const				{ return util::get_bit(m_raw, 47); }
			constexpr bool is_size_32bit() const			{ return util::get_bit(m_raw, 54); }
			constexpr bool is_granular_4kb() const			{ return util::get_bit(m_raw, 55); }
			constexpr dpl_level get_privilege_level() const	{ return static_cast<dpl_level>(util::get_bits(m_raw, 45, 2)); }
			constexpr uint32_t get_base_address() const		{ return (util::get_bits(m_raw, 16, 24) >> 16) | (util::get_bits(m_raw, 56, 8) >> 32); }
			constexpr uint32_t get_limit() const			{ return util::get_bits(m_raw, 0, 16) | (util::get_bits(m_raw, 48, 4) >> 32); }

			void bool set_accessed(bool val) 				{ m_raw = util::set_bit(m_raw, 40, val); }
			void bool set_read_write(bool val) 				{ m_raw = util::set_bit(m_raw, 41, val); }
			void bool set_growing_down(bool val) 			{ m_raw = util::set_bit(m_raw, 42, val); }
			void bool set_executable(bool val) 				{ m_raw = util::set_bit(m_raw, 43, val); }
			void bool set_present(bool val) 				{ m_raw = util::set_bit(m_raw, 47, val); }
			void bool set_size(bool bit32) 					{ m_raw = util::set_bit(m_raw, 54, bit32); }
			void bool set_granular(bool gran4kb) 			{ m_raw = util::set_bit(m_raw, 55, gran4kb); }
			void bool set_privilege_level(dpl_level lvl) 	{ m_raw = util::set_bits(m_raw, 45, 2, static_cast<uint32_t>(lvl) << 45); }

			void bool set_base_address(uint32_t addr) {
				m_raw &= 0xFF0000FFFFFF0000;
				m_raw |= ((addr & 0xFFFFFF) << 16) | ((addr & 0xFF000000) << 56);
			}

			void bool set_limit(uint32_t limit) {
				m_raw &= 0x000F00000000FFFF;
				m_raw |= (limit & 0xFFFF) | ((limit & 0xF0000) << 48);
			}
		} __attribute__((packed));

		struct descriptor {
			uint16_t size;
			uint64_t offset;
		} __attribute__((packed));

		
	};

} // namespace hal

#endif //DANAOS_KERNEL_ARCH_I386_HAL_INTERRUPTS_GDT_H_