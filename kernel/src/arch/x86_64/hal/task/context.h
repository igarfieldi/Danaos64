#ifndef DANAOS_KERNEL_ARCH_X86_64_HAL_TASK_CONTEXT_H_
#define DANAOS_KERNEL_ARCH_X86_64_HAL_TASK_CONTEXT_H_

#include <stdint.h>
#include "hal/interrupts/isr_arch.h"

namespace task {
    class task;
} // namespace task

namespace hal {


    struct thread_context {
        uint64_t ss;
        uint64_t rsp;

        thread_context(uint64_t ss, uint64_t rsp) : ss(ss), rsp(rsp) {}
        thread_context(const isr_frame_equal_priv &isr_frame) : ss(0), rsp(isr_frame.registers.rsp) {}
        void change_frame(isr_frame_equal_priv &isr_frame) const {
            isr_frame.registers.rsp = rsp;
            isr_frame.ret_rsp = rsp;
        }
    };

    thread_context create_context(uint64_t *stack, task::task &task);

} // namespace hal

#endif //DANAOS_KERNEL_ARCH_X86_64_HAL_TASK_CONTEXT_H_