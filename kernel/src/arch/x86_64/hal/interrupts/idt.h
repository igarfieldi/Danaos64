#ifndef DANAOS_KERNEL_ARCH_X86_64_HAL_INTERRUPTS_IDT_H_
#define DANAOS_KERNEL_ARCH_X86_64_HAL_INTERRUPTS_IDT_H_

#include <stdint.h>
#include "hal/interrupts/gdt.h"
#include "hal/util/bitmap.h"
#include "hal/interrupts/isr_arch.h"

namespace hal {

	class int_desc_table {
	public:
		static constexpr uint16_t GATE_COUNT = ISR_COUNT;

		enum class gate_type : uint8_t {
			CALL = 0b1100,
			INTERRUPT = 0b1110,
			TRAP = 0b1111
		};

		class gate {
		private:
			uint16_t m_offset_low;
			glob_desc_table::selector m_selector;
			uint8_t m_ist;
			uint8_t m_flags;
			uint16_t m_offset_mid;
			uint32_t m_offset_high;
			uint32_t m_reserved;
		
		public:
			gate() = default;
			gate(const gate &) = default;
			gate(gate &&) = default;
			gate &operator=(const gate &) = default;
			gate &operator=(gate &&) = default;
			~gate() = default;

			gate(uint64_t offset, glob_desc_table::selector sel, gate_type type, dpl_level lvl, uint8_t ist, bool present) :
					m_offset_low(offset & 0xFFFF), m_selector(sel),
					m_ist(ist & (type == gate_type::CALL ? 0x0 : 0x7)),
					m_flags((static_cast<uint8_t>(type) & 0xF) | ((static_cast<uint8_t>(lvl) & 0x3) << 5) | (present << 7)),
					m_offset_mid((offset >> 16) & 0xFFFFFFFF), m_offset_high(offset >> 32),
					m_reserved(0) {}

			constexpr bool is_present() const							{ return util::get_bit(m_flags, 7); }
			constexpr dpl_level get_privilege_level() const				{ return static_cast<dpl_level>((m_flags >> 5) & 0x3); }
			constexpr gate_type get_type() const						{ return static_cast<gate_type>(m_flags & 0xF); }
			constexpr uint8_t get_ist() const							{ return m_ist & 0x7; } // Only useful for Trap or Interrupt
			constexpr glob_desc_table::selector get_selector() const	{ return m_selector; }
			constexpr uint64_t get_offset() const						{ return m_offset_low | (m_offset_mid << 16) | (uint64_t{m_offset_high} << 32); }

			void set_present(bool val) 									{ m_flags = util::set_bit(m_flags, 7, val); }
			void set_privilege_level(dpl_level lvl) 					{ m_flags = util::set_bits<uint8_t>(m_flags, 5, 2, static_cast<uint8_t>(lvl) << 5); }
			void set_selector(glob_desc_table::selector sel) 			{ m_selector = sel; }

			void set_ist(uint8_t ist) {
				if(get_type() != gate_type::CALL) {
					m_ist = util::set_bits(m_ist, 0, 3, ist);
				}
			}
			void set_type(gate_type type) {
				if(type == gate_type::CALL) {
					m_reserved &= ~0x1F00;
				}
				m_flags = util::set_bits<uint8_t>(m_flags, 0, 4, static_cast<uint8_t>(type));
			}

			void set_offset(uint64_t offset) {
				m_offset_low = offset & 0xFFFF;
				m_offset_mid = (offset >> 16) & 0xFFFFFFFF;
				m_offset_high = offset >> 32;
			}
		} __attribute__((packed));

	private:
		struct descriptor {
			uint16_t size;
			uint64_t address;
		} __attribute__((packed));

		gate m_table[GATE_COUNT];
		descriptor m_descr;

		int_desc_table() noexcept;

	public:
		int_desc_table(const int_desc_table &) = delete;
		int_desc_table(int_desc_table &&) = delete;
		int_desc_table& operator=(const int_desc_table &) = delete;
		int_desc_table& operator=(int_desc_table &&) = delete;
		~int_desc_table() = default;	// TODO: replace with cleanup?

		static int_desc_table& instance() noexcept;

		void init(const glob_desc_table::selector &kernel_seg) noexcept;

		void set_gate(uint8_t index, gate gate) noexcept;
		void load() noexcept;
	};

} // namespace hal

#endif //DANAOS_KERNEL_ARCH_X86_64_HAL_INTERRUPTS_IDT_H_