#ifndef DANAOS_KERNEL_ARCH_PC_DEVICES_PIT_H_
#define DANAOS_KERNEL_ARCH_PC_DEVICES_PIT_H_

#include "hal/util/bitmap.h"
#include "hal/interrupts/interrupts.h"
#include "io/port.h"

namespace hal {

	// TODO: const-correctness?
	class interval_timer : public isr {
	private:
		// TODO: proper description?
		enum class operating_mode : uint8_t {
			MODE_0 = 0,
			MODE_1 = 2,
			MODE_2 = 4,
			MODE_3 = 6,
			MODE_4 = 8,
			MODE_5 = 10
		};

		enum class access_mode : uint8_t {
			LATCH_COUNT_VALUE = 0,
			LOW_ONLY          = 0x10,
			HIGH_ONLY         = 0x20,
			LOW_HIGH          = 0x30
		};

		enum class channel : uint8_t {
			CHANNEL_0 = 0,
			CHANNEL_1 = 0x40,
			CHANNEL_2 = 0x80
			// READ_BACK_CMD = 3     // TODO: enable this?
		};

		class command {
		private:
			uint8_t m_raw;

		public:
			command(bool bcd, operating_mode op, access_mode access, channel channel)
				: m_raw(bcd | static_cast<uint8_t>(op) | static_cast<uint8_t>(access)
						| static_cast<uint8_t>(channel)) {}

			constexpr bool is_bcd() const					{ return util::get_bit(m_raw, 0); }
			constexpr operating_mode get_op_mode() const	{ return static_cast<operating_mode>(m_raw & 0xE); }
			constexpr access_mode get_access_mode() const	{ return static_cast<access_mode>(m_raw & 0x30); }
			constexpr channel get_channel() const			{ return static_cast<channel>(m_raw & 0xC0); }

			void set_bcd(bool val)							{ m_raw = util::set_bit(m_raw, 0, val); }
			void set_op_mode(operating_mode val)			{ m_raw = util::set_bits(m_raw, 1, 3, static_cast<uint8_t>(val)); }
			void set_access_mode(access_mode val)			{ m_raw = util::set_bits(m_raw, 4, 2, static_cast<uint8_t>(val)); }
			void set_channel(channel val)					{ m_raw = util::set_bits(m_raw, 6, 2, static_cast<uint8_t>(val)); }

			constexpr uint8_t get_raw() const				{ return m_raw; }
		} __attribute__((packed));

		static constexpr uint16_t BASE_ADDRESS   = 0x40;
		static constexpr uint32_t BASE_FREQUENCY = 1193182;
		static constexpr uint16_t MIN_DIVISOR    = 2;
		static constexpr uint16_t MAX_DIVISOR    = 65535;
		static constexpr unsigned int DEFAULT_FREQUENCY = 1000;

		out_port<uint8_t> control;
		out_port<uint8_t> channel0;
		out_port<uint8_t> channel1;
		out_port<uint8_t> channel2;
		size_t counter;
		uint16_t currentDivisor;
		uint16_t currentFrequency;

	public:
		interval_timer() noexcept;
		interval_timer(const interval_timer &) = delete;
		interval_timer(interval_timer &&)      = delete;
		~interval_timer() noexcept;
		interval_timer &operator=(const interval_timer &) = delete;
		interval_timer &operator=(interval_timer &&) = delete;

		void init(unsigned int freq) noexcept;
		static interval_timer &instance() noexcept;

		virtual void enable() noexcept override;
		virtual void disable() noexcept override;
        virtual bool is_active() const noexcept override;

		unsigned int get_frequency() const noexcept;
		void set_frequency(unsigned int) noexcept;
		
		virtual void trigger(isr_frame &frame) noexcept override;
	};

}

#endif //DANAOS_KERNEL_ARCH_PC_DEVICES_PIT_H_