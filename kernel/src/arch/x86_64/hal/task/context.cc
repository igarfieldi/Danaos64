#include "context.h"
#include "hal/interrupts/gdt.h"
#include "main/task/task.h"

namespace hal {

    thread_context create_context(uint64_t *stack, task::task &task) {
        thread_context context;
        context.rbx = 0;
        context.r12 = 0;
        context.r13 = 0;
        context.r14 = 0;
        context.r15 = 0;
        context.rbp = reinterpret_cast<uint64_t>(&stack[0]);
        context.rsp = reinterpret_cast<uint64_t>(&stack[-2]);
        context.rip = reinterpret_cast<uint64_t>(&task::task::start);

        // New state
        stack[-2] = reinterpret_cast<uint64_t>(&task::task::start);	// Return EIP
        stack[-1] = 0xDEADCAFECAAAFEEE;			                    // Start_task return EIP (must not return!)
        stack[0] = reinterpret_cast<uint64_t>(&task);			    // Start_task function parameter
        
        return context;
    }

} // namespace hal