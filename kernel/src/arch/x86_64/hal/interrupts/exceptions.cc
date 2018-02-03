#include "exceptions.h"
#include "hal/devices/cpu_detail.h"

namespace hal {
    
    static cpu_exception exc0(0, "Division by Zero");
    static cpu_exception exc1(1, "Debug");
    static cpu_exception exc2(2, "Non-maskable Interrupt");
    static cpu_exception exc3(3, "Breakpoint");
    static cpu_exception exc4(4, "Overflow");
    static cpu_exception exc5(5, "Bound Range Exceeded");
    static cpu_exception exc6(6, "Invalid Opcode");
    static cpu_exception exc7(7, "Device Not Available");
    static cpu_exception exc8(8, "Double Fault");
    static cpu_exception exc9(9, "Coprocessor Segment Overrun");
    static cpu_exception exc10(10, "Invalid TSS");
    static segment_missing_exc exc11;
    static cpu_exception exc12(12, "Stack-Segment Fault");
    static gen_protection_fault exc13;
    static page_fault exc14;
    static cpu_exception exc16(16, "x87 Floating-Point Exception");
    static cpu_exception exc17(17, "Alignment Check");
    static cpu_exception exc18(18, "Machine Check");
    static cpu_exception exc19(19, "SIMD Floating-Point Exception");
    static cpu_exception exc20(20, "Virtualization Exception");
    static cpu_exception exc30(30, "Security Exception");

    const char *gen_protection_fault::TBL_NAMES[] = {
        "GDT", "IDT", "LDT", "IDT"
    };

    void install_exceptions() noexcept {
        exc0.enable();
        exc1.enable();
        exc2.enable();
        exc3.enable();
        exc4.enable();
        exc5.enable();
        exc6.enable();
        exc7.enable();
        exc8.enable();
        exc9.enable();
        exc10.enable();
        exc11.enable();
        exc12.enable();
        exc13.enable();
        exc14.enable();
        exc16.enable();
        exc17.enable();
        exc18.enable();
        exc19.enable();
        exc20.enable();
        exc30.enable();
    }

    void cpu_exception::enable() noexcept {
        hal::isr_dispatcher::instance().register_isr(m_index, *this);
        m_enabled = true;
    }

    void cpu_exception::disable() noexcept {
        hal::isr_dispatcher::instance().deregister_isr(m_index);
        m_enabled = false;
    }

    bool cpu_exception::is_active() const noexcept {
        return m_enabled;
    }

    void cpu_exception::trigger(isr_frame &frame) noexcept {
        kernel::m_console.print("CPU exception {} occured: {}\n"
                "  ESP:    []\n"
                "  EBP:    []\n"
                "  CS:     []\n"
                "  EIP:    []\n"
                "  EFLAGS: []\n",
            frame.int_num, m_name, frame.rsp, frame.rbp,
            frame.cs, frame.ret_rip, frame.rflags);
        kernel::panic("Faulting function: {}",
                kernel::m_elf_lookup.lookup(frame.ret_rip));
    }

    void segment_missing_exc::trigger(isr_frame &frame) noexcept {
        kernel::m_console.print("Missing segment {} was accessed!\n", frame.error_code);
        cpu_exception::trigger(frame);
    }

    void gen_protection_fault::trigger(isr_frame &frame) noexcept {
        if(frame.error_code != 0) {
            kernel::m_console.print("General protection fault occurred", frame.error_code);
        } else {
            selector_err_code code;
            std::memcpy(&code, &frame.error_code, sizeof(selector_err_code));
            kernel::m_console.print("General protection fault!\n"
                    "  External: {}\n  Table: {}\n  Table index: {}",
                    code.external, TBL_NAMES[code.table], code.index);
        }
        cpu_exception::trigger(frame);
    }

    void page_fault::trigger(isr_frame &frame) noexcept {
        page_error_code code;
        std::memcpy(&code, &frame.error_code, sizeof(page_error_code));
        kernel::m_console.print("Page fault!\n"
                "  Present: {}\n  On write: {}\n  User: {}\n  Reserved write: {}\n"
                "  Instruction fetch: {}\n  Address: []",
            code.present, code.write, code.user, code.res_write, code.instr_fetch,
            hal::cpu::read_cr2());
        cpu_exception::trigger(frame);
    }

} // namespace hal