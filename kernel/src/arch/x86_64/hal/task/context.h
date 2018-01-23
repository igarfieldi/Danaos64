#ifndef DANAOS_KERNEL_ARCH_X86_64_HAL_TASK_CONTEXT_H_
#define DANAOS_KERNEL_ARCH_X86_64_HAL_TASK_CONTEXT_H_

#include <stdint.h>
#include "hal/interrupts/isr_arch.h"

namespace task {
    class task;
} // namespace task

namespace hal {

    struct thread_context {
        uint64_t rbx;
        uint64_t rbp;
        uint64_t r12;
        uint64_t r13;
        uint64_t r14;
        uint64_t r15;
        uint64_t rsp;
    };

    thread_context create_context(uint64_t *stack, task::task &task);

} // namespace hal

extern "C" void switch_context(hal::thread_context *curr, hal::thread_context *next);
extern "C" void start_context();

namespace task {

    struct task_state_segment {
        uint32_t reserved_1;
        uint64_t rsp0;
        uint64_t rsp1;
        uint64_t rsp2;
        uint64_t reserved_2;
        uint64_t ist1;
        uint64_t ist2;
        uint64_t ist3;
        uint64_t ist4;
        uint64_t ist5;
        uint64_t ist6;
        uint64_t ist7;
        uint64_t reserved_3;
        uint16_t reserved_4;
        uint16_t iopb_offset;
    } __attribute__((packed));

} // namespace task

#endif //DANAOS_KERNEL_ARCH_X86_64_HAL_TASK_CONTEXT_H_
