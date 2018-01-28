#ifndef DANAOS_HAL_INTERRUPTS_INTERRUPTS_H_
#define DANAOS_HAL_INTERRUPTS_INTERRUPTS_H_

#include <stddef.h>
#include "hal/interrupts/isr_count.h"
#include "main/kernel.h"

namespace hal {

	class isr {
	public:
		virtual ~isr() {}

		virtual void enable() noexcept = 0;
		virtual void disable() noexcept = 0;
		virtual bool is_active() const noexcept = 0;

		virtual void trigger() noexcept = 0;
	};

	class test_isr : public isr {
	public:
		virtual ~test_isr() {}
		
		virtual void enable() noexcept {}
		virtual void disable() noexcept {}
		virtual bool is_active() const noexcept {return true;}

		virtual void trigger() noexcept {
			kernel::m_console.print("Triggered!\n");
		}
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

		void trigger(size_t index) const;
	};

} // namespace hal

#endif //DANAOS_HAL_INTERRUPTS_INTERRUPTS_H_