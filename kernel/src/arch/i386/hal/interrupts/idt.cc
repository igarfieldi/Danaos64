#include "idt.h"
#include "main/kernel.h"

extern void (*_isr_addresses[hal::int_desc_table::GATE_COUNT])();

extern "C" void isr_handler(void *isr_state) {
	kernel::m_console.print("Interrupt called!\n");
	while(true);
}

namespace hal {

	int_desc_table::int_desc_table() noexcept : m_descr{
			static_cast<uint16_t>(sizeof(gate) * GATE_COUNT - 1),
        	reinterpret_cast<uint32_t>(&m_table[0])
		} {}
	

	int_desc_table& int_desc_table::instance() noexcept {
		static int_desc_table instance;
		return instance;
	}

	void int_desc_table::init(const glob_desc_table::selector &kernel_seg) noexcept {
		kernel::m_console.print("Initializing IDT...\n");
		for (size_t i = 0; i < GATE_COUNT; i++) {
			m_table[i] = gate(reinterpret_cast<uint32_t>(_isr_addresses[i]),
							kernel_seg, gate_type::INTERRUPT, dpl_level::RING_0, true);
		}

		this->load();
	}

    void int_desc_table::set_gate(uint8_t index, gate gate) noexcept {
		// No check for validity needed; 256 gates means 1 byte holds exactly that
		m_table[index] = gate;
	}

	void int_desc_table::load() noexcept {
		__asm__ volatile("lidt (%0)" : : "d"(reinterpret_cast<uint32_t>(&m_descr)));
	}

} // namespace hal