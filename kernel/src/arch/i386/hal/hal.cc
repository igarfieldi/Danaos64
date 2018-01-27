#include "hal/hal.h"
#include "hal/interrupts/gdt.h"
#include "main/kernel.h"

namespace hal {

	void init_hal() {
		kernel::m_console.print("Initializing HAL...\n");
		glob_desc_table::instance().init();
	}

} // namespace hal