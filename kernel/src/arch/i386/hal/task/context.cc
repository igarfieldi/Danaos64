#include "context.h"
#include "hal/interrupts/gdt.h"
#include "main/task/task.h"

namespace hal {

    thread_context create_context(uint32_t *stack, task::task &task) {
        thread_context context;
        context.ebx = 0;
        context.edi = 0;
        context.esi = 0;
        context.ebp = reinterpret_cast<uint64_t>(&stack[0]);
        context.esp = reinterpret_cast<uint64_t>(&stack[-2]);

        // New state
        stack[-2] = reinterpret_cast<uint64_t>(&task::task::start);		// Start function
        stack[-1] = 0xCAFEBABE;											// Return address of start function (musn't be called!)
        stack[0] = reinterpret_cast<uint64_t>(&task);			    	// Start function parameter
        
        return context;
    }

} // namespace hal
