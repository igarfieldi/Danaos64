#ifndef DANAOS_KERNEL_ARCH_X86_64_HAL_TASK_CONTEXT_H_
#define DANAOS_KERNEL_ARCH_X86_64_HAL_TASK_CONTEXT_H_

#include <stdint.h>
#include "hal/interrupts/isr_arch.h"

namespace task {
    class task;
} // namespace task

namespace hal {

    class task_context {
    private:
        uint64_t rsp;
        
    public:
       	explicit task_context(volatile uintptr_t *stack) : rsp(reinterpret_cast<uint64_t>(stack)) {}
       	explicit task_context(uintptr_t *stack) : rsp(reinterpret_cast<uint64_t>(stack)) {}
       	explicit task_context(const hal::isr_frame &frame) : rsp(frame.rsp) {}
       	
       	void switch_frame(hal::isr_frame &frame) const {
       		frame.rsp = rsp;
       	}
    };

    task_context create_context(volatile uint64_t *stack, task::task &task);

} // namespace hal

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
