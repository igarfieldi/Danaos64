#include "hal/hal.h"
#include "main/kernel.h"

namespace hal {

	void init_hal() {
		kernel::m_console.print("Initializing HAL...\n");
	}

} // namespace hal