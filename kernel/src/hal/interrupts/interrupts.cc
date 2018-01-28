#include "interrupts.h"
#include "main/kernel.h"

extern "C" void isr_handler(void *isr_state) {
	(void) isr_state;
	size_t index = reinterpret_cast<uint32_t*>(isr_state)[3];
	hal::isr_dispatcher::instance().trigger(index);
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