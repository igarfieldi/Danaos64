#include "pit.h"
#include "hal/interrupts/interrupts.h"
#include "main/task/scheduler.h"
#include "devices/pic.h"
#include "libk/math.h"

namespace hal {

	interval_timer::interval_timer() noexcept
		: control(BASE_ADDRESS + 3),
		channel0(BASE_ADDRESS),
		channel1(BASE_ADDRESS + 1),
		channel2(BASE_ADDRESS + 2),
		currentDivisor(0),
		currentFrequency(0) {
	}

	interval_timer::~interval_timer() noexcept {
		//this->disable();
	}

	void interval_timer::init(unsigned int freq) noexcept {
		this->set_frequency(freq);
		this->enable();
	}
	
	interval_timer &interval_timer::instance() noexcept {
		static interval_timer instance;
		return instance;
	}

	void interval_timer::enable() noexcept {
		hal::irq_controller::instance().enable(hal::irq_controller::irq::PIT, *this);
	}

	void interval_timer::disable() noexcept {
		hal::irq_controller::instance().disable(hal::irq_controller::irq::PIT);
	}
	
    bool interval_timer::is_active() const noexcept {
		return hal::irq_controller::instance().is_enabled(hal::irq_controller::irq::PIT);
	}

	/*void IntervalTimer::trigger(InterruptState *state) {
		// TODO: feels too slow (caused by emulator)
		currentTick++;
		std::string time = std::string("PIT: ") + std::to_string(getCurrentMillisecond()) + "ms";

		for (size_t i = 0; i < std::strlen(time.data()); i++) {
			Kernel::cga.print(80 - std::strlen(time.data()) + i, 1, time.data()[i]);
		}
	}*/

	unsigned int interval_timer::get_frequency() const noexcept {
		return currentFrequency;
	}

	void interval_timer::set_frequency(unsigned int freq) noexcept {
		currentDivisor =
			math::max<uint16_t>(math::min<uint32_t>(BASE_FREQUENCY / freq, MAX_DIVISOR), MIN_DIVISOR);
		currentFrequency = BASE_FREQUENCY / static_cast<uint32_t>(currentDivisor);

		asm volatile("cli");
		//bool previous = Kernel::cpu.disableInterrupts();
		control.write(command(0, operating_mode::MODE_2, access_mode::LOW_HIGH, channel::CHANNEL_0).get_raw());
		channel0.write(static_cast<uint8_t>(currentDivisor & 0xFF));
		channel0.write(static_cast<uint8_t>((currentDivisor >> 8) & 0xFF));

		asm volatile("sti");
		/*if(previous) {
			Kernel::cpu.enableInterrupts();
		}*/
	}

	void interval_timer::trigger(isr_frame &frame) noexcept  {
		const hal::task_context &next_context = task::scheduler::instance().schedule(hal::task_context(frame));
		next_context.switch_frame(frame);
	}

} //namespace hal