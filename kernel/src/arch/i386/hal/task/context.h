#ifndef DANAOS_KERNEL_ARCH_I386_HAL_TASK_CONTEXT_H_
#define DANAOS_KERNEL_ARCH_I386_HAL_TASK_CONTEXT_H_

#include <stdint.h>
#include "hal/interrupts/isr_arch.h"

namespace task {
    class task;
} // namespace task

namespace hal {

    class task_context {
    private:
        uint32_t esp;
        
    public:
       	explicit task_context(volatile uintptr_t *stack) : esp(reinterpret_cast<uint32_t>(stack)) {}
       	explicit task_context(uintptr_t *stack) : esp(reinterpret_cast<uint32_t>(stack)) {}
       	explicit task_context(const hal::isr_frame &frame) : esp(frame.esp) {}
       	
       	void switch_frame(hal::isr_frame &frame) const {
       		frame.esp = esp;
       	}
    };

    task_context create_context(volatile uint32_t *stack, task::task &task);

} // namespace hal

extern "C" void switch_context(hal::task_context *curr, hal::task_context *next);

namespace task {

    struct task_state_segment {
        struct segment {
        private:
            uint16_t seg;
            uint16_t reserved;

        public:
            segment() = default;
            segment(uint16_t seg) : seg(seg), reserved(0) {}
            segment &operator=(uint16_t seg)    { this->seg = seg; return *this; }
            operator uint16_t() const           { return seg; }
        } __attribute__((packed));

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
