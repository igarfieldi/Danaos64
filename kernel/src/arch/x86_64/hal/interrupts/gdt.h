#ifndef DANAOS_KERNEL_ARCH_X86_64_HAL_INTERRUPTS_GDT_H_
#define DANAOS_KERNEL_ARCH_X86_64_HAL_INTERRUPTS_GDT_H_

#include <stdint.h>
#include "hal/util/bitmap.h"

namespace hal {

	enum class dpl_level : uint8_t {
		RING_0 = 0,
		RING_1 = 1,
		RING_2 = 2,
		RING_3 = 3
	};

	class glob_desc_table {
	public:
		static constexpr uint16_t INVALID_INDEX = 65535;

		class entry {
		private:
			uint64_t m_raw;

		public:
			entry() = default;
			entry(const entry &) = default;
			entry(entry &&) = default;
			entry &operator=(const entry &) = default;
			entry &operator=(entry &&) = default;
			~entry() = default;

			entry(uint64_t raw) noexcept : m_raw(raw) {}
			entry(uint32_t base, uint32_t limit, dpl_level priv, bool present, bool rw, bool down,
					bool exec, bool gran4kb, bool longmode) noexcept :
						m_raw((limit & 0xFFFF) | (uint64_t{base & 0xFFFFFF} << 16)
							| (uint64_t{rw} << 41) | (uint64_t{down} << 42) | (uint64_t{exec} << 43)
							| (uint64_t{longmode} << 44) | (static_cast<uint64_t>(priv) << 45)
							| (uint64_t{present} << 47) | (uint64_t{limit & 0xF0000} << 32)
							| (uint64_t{1} << 53) | (uint64_t{gran4kb} << 55)
							| (uint64_t{base & 0xFF000000} << 32)) {}

			constexpr bool is_accessed() const				{ return util::get_bit(m_raw, 40); }
			constexpr bool is_read_write() const			{ return util::get_bit(m_raw, 41); }
			constexpr bool is_growing_down() const			{ return util::get_bit(m_raw, 42); }
			constexpr bool is_executable() const			{ return util::get_bit(m_raw, 43); }
			constexpr bool is_longmode() const				{ return util::get_bit(m_raw, 44); }
			constexpr bool is_present() const				{ return util::get_bit(m_raw, 47); }
			constexpr bool is_granular_4kb() const			{ return util::get_bit(m_raw, 55); }
			constexpr dpl_level get_privilege_level() const	{ return static_cast<dpl_level>(util::get_bits(m_raw, 45, 2)); }
			constexpr uint32_t get_base_address() const		{ return (util::get_bits(m_raw, 16, 24) >> 16) | (util::get_bits(m_raw, 56, 8) >> 32); }
			constexpr uint32_t get_limit() const			{ return util::get_bits(m_raw, 0, 16) | (util::get_bits(m_raw, 48, 4) >> 32); }

			void set_accessed(bool val) 					{ m_raw = util::set_bit(m_raw, 40, val); }
			void set_read_write(bool val) 					{ m_raw = util::set_bit(m_raw, 41, val); }
			void set_growing_down(bool val) 				{ m_raw = util::set_bit(m_raw, 42, val); }
			void set_executable(bool val) 					{ m_raw = util::set_bit(m_raw, 43, val); }
			void set_longmode(bool val) 					{ m_raw = util::set_bit(m_raw, 44, val); }
			void set_present(bool val) 						{ m_raw = util::set_bit(m_raw, 47, val); }
			void set_granular(bool gran4kb) 				{ m_raw = util::set_bit(m_raw, 55, gran4kb); }
			void set_privilege_level(dpl_level lvl) 		{ m_raw = util::set_bits(m_raw, 45, 2, static_cast<uint64_t>(lvl) << 45); }

			void set_base_address(uint32_t addr) {
				m_raw &= 0xFF0000FFFFFF0000;
				m_raw |= (uint64_t{addr & 0xFFFFFF} << 16) | (uint64_t{addr & 0xFF000000} << 32);
			}

			void set_limit(uint32_t limit) {
				m_raw &= 0x000F00000000FFFF;
				m_raw |= (limit & 0xFFFF) | (uint64_t{limit & 0xF0000} << 32);
			}
		} __attribute__((packed));
		
		class selector {
		private:
			uint16_t m_raw;

		public:
			selector() = default;
			selector(const selector &) = default;
			selector(selector &&) = default;
			selector &operator=(const selector &) = default;
			selector &operator=(selector &&) = default;
			~selector() = default;

			selector(uint16_t raw) : m_raw(raw) {}
			selector(dpl_level priv, bool local, uint16_t index) :
					m_raw(static_cast<uint16_t>(priv) | (local << 2) | ((index & 0x1FFF) << 3)) {}

			constexpr bool is_local() const					{ return util::get_bit(m_raw, 2); }
			constexpr dpl_level get_privilege_level() const	{ return static_cast<dpl_level>(util::get_bits(m_raw, 0, 2)); }
			constexpr uint16_t get_index() const			{ return (m_raw & 0xFFF8) << 3; }
			constexpr uint16_t get_segment_selector() const	{ return (m_raw & 0xFFF8); }

			void set_local(bool local) 						{ m_raw = util::set_bit(m_raw, 2, local); }
			void set_privilege_level(dpl_level lvl) 		{ m_raw = util::set_bits(m_raw, 0, 2, static_cast<uint16_t>(lvl)); }
			void set_index(uint16_t index) 					{ m_raw &= 0x7; m_raw |= ((index & 0x1FFF) << 3); }
			void set_segment_selector(uint16_t sel) 		{ m_raw &= 0x7; m_raw |= (sel & 0x1FFF); }
		} __attribute__((packed));

	private:
		static constexpr uint16_t KERNEL_CODE_INDEX = 1;
		static constexpr uint16_t KERNEL_DATA_INDEX = 2;
		static constexpr uint16_t USER_CODE_INDEX = 3;
		static constexpr uint16_t USER_DATA_INDEX = 4;

		struct descriptor {
			uint16_t size;
			uint64_t offset;
		} __attribute__((packed));

		uint16_t m_capacity, m_index;
		entry *m_entries;
		descriptor m_descriptor;

		glob_desc_table() noexcept;

	public:
		glob_desc_table(const glob_desc_table &) = delete;
		glob_desc_table(glob_desc_table &&) = delete;
		glob_desc_table &operator=(const glob_desc_table &) = delete;
		glob_desc_table &operator=(glob_desc_table &&) = delete;
		~glob_desc_table() noexcept;

		static glob_desc_table &instance() noexcept;
		void init() noexcept;

		uint16_t push_entry(entry entry) noexcept;
		void set_entry(entry entry, uint16_t index) noexcept;
		entry &get_entry(uint16_t index) const noexcept;

		selector get_selector(uint16_t index) const noexcept;
		selector get_kernel_selector() const noexcept;

		void load() noexcept;
	};

} // namespace hal

#endif //DANAOS_KERNEL_ARCH_X86_64_HAL_INTERRUPTS_GDT_H_
