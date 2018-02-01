#include "semaphore.h"
#include "main/task/scheduler.h"
#include "main/kernel.h"

namespace sync {

	semaphore::semaphore() : m_count(1), m_queue() {}

	void semaphore::wait() {
		//cpu.disable_int();
		kernel::m_console.print("Count: {}\n", m_count);
		if(m_count > 0) {
			m_count = 0;
			//cpu.enable_int();//cpu.restore_interrupt_state();
		} else {
			m_queue.enqueue(&task::scheduler::instance().active_task());
			task::scheduler::instance().block();
		}
	}

	void semaphore::signal() {
		//cpu.disable_int();
		if(m_queue.is_empty()) {
			m_count = 1;
		} else {
			m_queue.dequeue()->ready();
		}
		//cpu.enable_int();//cpu.restore_interrupt_state();
	}

} // namespace sync