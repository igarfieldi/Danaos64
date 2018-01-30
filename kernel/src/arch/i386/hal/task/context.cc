#include "context.h"
#include "hal/interrupts/gdt.h"
#include "main/task/task.h"

namespace hal {

    thread_context create_context(uint32_t *stack, task::task &task) {
        thread_context context(0, reinterpret_cast<uint32_t>(&stack[-9]));
        // TODO: proper SS

        // New state
        stack[-9] = 0xCAFEBABE;									// EAX
        stack[-8] = 0xFAFAFAFA;									// ECX
        stack[-7] = 0xDEADBEEF;									// EDX
        stack[-6] = 0;											// Interrupt number
        stack[-5] = 0;											// Error code
        stack[-4] = reinterpret_cast<uint32_t>(&task::task::start);	// Return EIP
        stack[-3] = hal::glob_desc_table::instance()
                            .get_kernel_selector()
                            .get_segment_selector();			// CS
        stack[-2] = 0;							                // EFLAGS
        stack[-1] = 0xCAAAFEEE;			                        // Start_task return EIP (must not return!)
        stack[0] = reinterpret_cast<uint32_t>(&task);			// Start_task function parameter

        __asm__ volatile("pushfl \n\t"
                "popl %%eax\n\t"
                "movl %%eax, %0 \n\t"
                : "=m"(stack[-2])
                :
                : "%eax"
        );
        
        return context;
    }

} // namespace hal