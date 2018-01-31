#ifndef DANAOS_KERNEL_ARCH_I386_HAL_TASK_CONTEXT_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_TASK_CONTEXT_H_

#include <stdint.h>
#include "hal/interrupts/isr_arch.h"

namespace task {
    class task;
} // namespace task

namespace hal {


    struct thread_context {
        uint32_t ss;
        uint32_t esp;

        thread_context(uint32_t ss, uint32_t esp) : ss(ss), esp(esp) {}
        thread_context(const isr_frame_equal_priv &isr_frame) : ss(0), esp(isr_frame.registers.esp) {}
        void change_frame(isr_frame_equal_priv &isr_frame) const {
            isr_frame.registers.esp = esp;
        }
    };

    thread_context create_context(uint32_t *stack, task::task &task);

} // namespace hal

namespace task {

    struct task_state_segment {
        class segment {
        private:
            uint16_t seg;
            uint16_t reserved;

        public:
            segment() = default;
            segment(uint16_t seg) : seg(seg), reserved(0) {}
            segment &operator=(uint16_t seg)    { this->seg = seg; }
            operator uint16_t() const           { return seg; }
        }

        segment link;
        uint32_t esp0;
        segment ss0;
        uint32_t esp1;
        segment ss1;
        uint32_t esp2;
        segment ss2;
        uint32_t cr3;
        uint32_t eip;
        uint32_t eflags;
        uint32_t eax;
        uint32_t ecx;
        uint32_t edx;
        uint32_t ebx;
        uint32_t esp;
        uint32_t ebp;
        uint32_t esi;
        uint32_t edi;
        segment es;
        segment cs;
        segment ss;
        segment ds;
        segment fs;
        segment gs;
        segment ldtr;
        uint16_t iopb_offset;
        uint16_t reserved;
    } __attribute__((packed));

} // namespace task

#endif //DANAOS_KERNEL_ARCH_I386_HAL_TASK_CONTEXT_H_