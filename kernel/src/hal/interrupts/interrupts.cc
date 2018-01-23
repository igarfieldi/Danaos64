#include "interrupts.h"
#include "main/kernel.h"
#include "hal/task/context.h"
#include "main/task/scheduler.h"



extern "C" void isr_handler(hal::isr_frame *frame) {
	kernel::m_console.print("Interrupt no.: [] - Error: []\n", frame->int_num, frame->error_code);
	
	const hal::task_context &next_context = task::scheduler::instance().schedule(hal::task_context(*frame));
	next_context.switch_frame(*frame);

	//hal::isr_dispatcher::instance().trigger(frame->int_num);
}

namespace hal {

	isr_dispatcher::isr_dispatcher() {
		for(size_t i = 0; i < ISR_COUNT; ++i) {
			m_isrs[i] = nullptr;
		}
	}

	isr_dispatcher::~isr_dispatcher() {
		for(size_t i = 0; i < ISR_COUNT; ++i) {
			if(m_isrs[i] != nullptr) {
				m_isrs[i]->disable();
			}
		}
	}
	
	isr_dispatcher &isr_dispatcher::instance() noexcept {
		static isr_dispatcher instance;
		return instance;
	}

	void isr_dispatcher::register_isr(size_t index, isr &isr) noexcept {
		if(index >= ISR_COUNT) {
			kernel::panic("Invalid ISR index!");
		}
		m_isrs[index] = &isr;
	}

	void isr_dispatcher::deregister_isr(size_t index) noexcept {
		if(index >= ISR_COUNT) {
			kernel::panic("Invalid ISR index!");
		}
		m_isrs[index] = nullptr;
	}

	void isr_dispatcher::trigger(size_t index) const {
		if(index >= ISR_COUNT) {
			kernel::panic("Invalid ISR index!");
		}
		if(m_isrs[index] == nullptr) {
			kernel::panic("Missing ISR handler!");
		}
		m_isrs[index]->trigger();
	}

} // namespace hal
