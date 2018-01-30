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

#endif //DANAOS_KERNEL_ARCH_I386_HAL_TASK_CONTEXT_H_