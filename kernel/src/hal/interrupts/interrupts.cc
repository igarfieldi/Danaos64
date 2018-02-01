#include "interrupts.h"
#include "main/kernel.h"
#include "hal/task/context.h"
#include "main/task/scheduler.h"

extern "C" void isr_handler(hal::isr_frame *frame) {
	//kernel::m_console.print("Interrupt no.: [] - Error: []\n", frame->int_num, frame->error_code);
	hal::isr_dispatcher::instance().trigger(*frame);
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

	void isr_dispatcher::trigger(isr_frame &frame) const {
		if(frame.int_num >= ISR_COUNT) {
			kernel::panic("Invalid ISR index: {}", frame.int_num);
		}
		if(m_isrs[frame.int_num] == nullptr) {
			kernel::panic("Missing ISR handler: {}", frame.int_num);
		}
		m_isrs[frame.int_num]->trigger(frame);
	}

} // namespace hal
