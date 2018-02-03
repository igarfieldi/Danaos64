#ifndef DANAOS_KERNEL_ARCH_X86_64_HAL_INTERRUPTS_EXCEPTIONS_H_
#define DANAOS_KERNEL_ARCH_X86_64_HAL_INTERRUPTS_EXCEPTIONS_H_

#include "hal/interrupts/interrupts.h"
#include "main/kernel.h"
#include "libk/string.h"

namespace hal {

    void install_exceptions() noexcept;

    class cpu_exception : public isr {
    private:
        const char *m_name;
        const size_t m_index;
        bool m_enabled;

    public:
        cpu_exception(size_t index, const char *name) : m_name(name), m_index(index), m_enabled(false) {}
        ~cpu_exception() {}

		virtual void enable() noexcept;
		virtual void disable() noexcept;
		virtual bool is_active() const noexcept;
		virtual void trigger(isr_frame &frame) noexcept;
    };

    class segment_missing_exc : public cpu_exception {
    public:
        segment_missing_exc() : cpu_exception(11, "Segment Not Present") {}
        ~segment_missing_exc() {}
		virtual void trigger(isr_frame &frame) noexcept;
    };

    class gen_protection_fault : public cpu_exception {
    private:
        struct selector_err_code {
            uint32_t external       : 1;
            uint32_t table          : 2;
            uint32_t index          : 13;
            uint32_t                : 16;
        } __attribute__((packed));

        static const char *TBL_NAMES[];
    public:
        gen_protection_fault() : cpu_exception(13, "General Protection Fault") {}
        ~gen_protection_fault() {}
		virtual void trigger(isr_frame &frame) noexcept;
    };

    class page_fault : public cpu_exception {
    private:
        struct page_error_code {
            uint32_t present        : 1;
            uint32_t write          : 1;
            uint32_t user           : 1;
            uint32_t res_write      : 1;
            uint32_t instr_fetch    : 1;
            uint32_t                : 27;
        } __attribute__((packed));

    public:
        page_fault() : cpu_exception(14, "Page fault") {}
        ~page_fault() {}
		virtual void trigger(isr_frame &frame) noexcept;
    };

} // namespace hal

#endif //DANAOS_KERNEL_ARCH_X86_64_HAL_INTERRUPTS_EXCEPTIONS_H_