#include "idt.h"
#include "main/kernel.h"
#include "hal/interrupts/interrupts.h"

extern void (*_isr_addresses[hal::int_desc_table::GATE_COUNT])();

namespace hal {

	int_desc_table::int_desc_table() noexcept : m_descr{
			static_cast<uint16_t>(sizeof(gate) * GATE_COUNT - 1),
        	reinterpret_cast<uint64_t>(&m_table[0])
		} {}
	

	int_desc_table& int_desc_table::instance() noexcept {
		static int_desc_table instance;
		return instance;
	}

	void int_desc_table::init(const glob_desc_table::selector &kernel_seg) noexcept {
		kernel::m_console.print("Initializing IDT...\n");
		for (size_t i = 0; i < GATE_COUNT; i++) {
			m_table[i] = gate(reinterpret_cast<uint64_t>(_isr_addresses[i]),
							kernel_seg, gate_type::INTERRUPT, dpl_level::RING_0, 0, true);
		}

		this->load();
	}

	void int_desc_table::load() noexcept {
		__asm__ volatile("lidt (%0)" : : "d"(reinterpret_cast<uint64_t>(&m_descr)));

		kernel::m_console.print("  IDT address  : []\n", m_descr.address);
		kernel::m_console.print("  IDT size     : {}\n", m_descr.size);
	}

} // namespace hal