#ifndef DANAOS_KERNEL_ARCH_I386_HAL_INTERRUPTS_IDT_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_INTERRUPTS_IDT_H_

#include <stdint.h>
#include "hal/interrupts/gdt.h"
#include "hal/util/bitmap.h"

namespace hal {

	class int_desc_table {
	public:
		static constexpr uint16_t GATE_COUNT = 256;

		enum class gate_type : uint8_t {
			INTERRUPT = 14,
			TRAP = 15
		};

		class gate {
		private:
			uint16_t m_offset_low;
			glob_desc_table::selector m_selector;
			uint8_t m_reserved;
			uint8_t m_flags;
			uint16_t m_offset_high;
		
		public:
			gate() = default;
			gate(const gate &) = default;
			gate(gate &&) = default;
			gate &operator=(const gate &) = default;
			gate &operator=(gate &&) = default;
			~gate() = default;

			gate(uint32_t offset, glob_desc_table::selector sel, gate_type type, dpl_level lvl, bool present) :
					m_offset_low(offset & 0xFFFF), m_selector(sel),
					m_reserved(0),
					m_flags((static_cast<uint8_t>(type) & 0xF) | ((static_cast<uint8_t>(lvl) & 0x3) << 5) | (present << 7)),
					m_offset_high(offset >> 16) {}

			constexpr bool is_present() const							{ return util::get_bit(m_flags, 7); }
			constexpr dpl_level get_privilege_level() const				{ return static_cast<dpl_level>((m_flags >> 5) & 0x3); }
			constexpr gate_type get_type() const						{ return static_cast<gate_type>(m_flags & 0xF); }
			constexpr glob_desc_table::selector get_selector() const	{ return m_selector; }
			constexpr uint32_t get_offset() const						{ return m_offset_low | (m_offset_high << 16); }

			void set_present(bool val) 									{ m_flags = util::set_bit(m_flags, 7, val); }
			void get_privilege_level(dpl_level lvl) 					{ m_flags = util::set_bits<uint8_t>(m_flags, 5, 2, static_cast<uint8_t>(lvl) << 5); }
			void get_type(gate_type type) 								{ m_flags = util::set_bits<uint8_t>(m_flags, 0, 4, static_cast<uint8_t>(type)); }
			void get_selector(glob_desc_table::selector sel) 			{ m_selector = sel; }
			void get_offset(uint32_t offset) 							{ m_offset_low = offset & 0xFFFF; m_offset_high = offset >> 16; }
		} __attribute__((packed));

	private:
		struct descriptor {
			uint16_t size;
			uint32_t address;
		} __attribute__((packed));

		static const uint32_t ISR_WRAPPER_SIZE;

		// TODO: keep on heap or on stack (or pre-allocate)?
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

#endif //DANAOS_KERNEL_ARCH_I386_HAL_INTERRUPTS_IDT_H_