#include "context.h"
#include "hal/interrupts/gdt.h"
#include "main/task/task.h"

namespace hal {

    task_context create_context(volatile uint32_t *stack, task::task &task) {
        task_context context(&stack[-10]);

        // New state
        stack[-10] = reinterpret_cast<uint64_t>(&stack[0]);				// EBP
        stack[-9] = 0xAAAAAAAA;											// EAX
        stack[-8] = 0xCCCCCCCC;											// ECX
        stack[-7] = 0xDDDDDDDD;											// EDX
        stack[-6] = 0x0;												// Interrupt number
        stack[-5] = 0x0;												// Error code
        stack[-4] = reinterpret_cast<uint64_t>(&task::task::start);		// Return EIP
        stack[-3] = 0x8;												// Return CS
        stack[-2] = 0x0;                                                // EFLAGS
        stack[-1] = 0xCAFEBABE;											// Return address of start function (musn't be called!)
        stack[0] = reinterpret_cast<uint64_t>(&task);			    	// Start function parameter
        
        return context;
    }

} // namespace hal
