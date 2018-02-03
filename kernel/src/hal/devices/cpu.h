#ifndef DANAOS_KERNEL_HAL_DEVICES_CPU_H_
#define DANAOS_KERNEL_HAL_DEVICES_CPU_H_

namespace hal {

	namespace cpu {

		struct int_state;

		void halt() noexcept;
		void enable_int() noexcept;
		void disable_int() noexcept;
		int_state get_enable_int() noexcept;
		int_state get_disable_int() noexcept;
		void restore_int(const int_state &state) noexcept;

	} // namespace cpu

} // namespace hal

#endif //DANAOS_KERNEL_HAL_DEVICES_CPU_H_