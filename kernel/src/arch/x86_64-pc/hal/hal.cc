#include "hal/hal.h"
#include "hal/interrupts/gdt.h"
#include "hal/interrupts/idt.h"
#include "devices/pic.h"
#include "devices/pit.h"
#include "main/kernel.h"
#include "hal/devices/cpu.h"
#include "hal/interrupts/exceptions.h"

namespace hal {

	void init_hal() noexcept {
		kernel::m_console.print("Initializing HAL...\n");
		glob_desc_table::instance().init();
		int_desc_table::instance().init(glob_desc_table::instance().get_kernel_selector());
		install_exceptions();
		irq_controller::instance().enable();
		interval_timer::instance().init(1);
		cpu::enable_int();
	}

} // namespace hal