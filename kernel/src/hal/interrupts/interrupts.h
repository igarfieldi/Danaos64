#ifndef DANAOS_HAL_INTERRUPTS_INTERRUPTS_H_
#define DANAOS_HAL_INTERRUPTS_INTERRUPTS_H_

#include <stddef.h>
#include "hal/interrupts/isr_arch.h"
#include "main/kernel.h"

namespace hal {

	class isr {
	public:
		virtual ~isr() {}

		virtual void enable() noexcept = 0;
		virtual void disable() noexcept = 0;
		virtual bool is_active() const noexcept = 0;

		virtual void trigger(isr_frame &frame) noexcept = 0;
	};

	class isr_dispatcher {
	private:
		isr *m_isrs[ISR_COUNT];

		isr_dispatcher();

	public:
		isr_dispatcher(const isr_dispatcher &) = delete;
		isr_dispatcher(isr_dispatcher &&) = delete;
		isr_dispatcher& operator=(const isr_dispatcher &) = delete;
		isr_dispatcher& operator=(isr_dispatcher &&) = delete;
		~isr_dispatcher();

		static isr_dispatcher &instance() noexcept;

		void register_isr(size_t index, isr &isr) noexcept;
		void deregister_isr(size_t index) noexcept;

		void trigger(isr_frame &frame) const;
	};

} // namespace hal

#endif //DANAOS_HAL_INTERRUPTS_INTERRUPTS_H_