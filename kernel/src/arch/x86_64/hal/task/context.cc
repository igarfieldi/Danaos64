#include "context.h"
#include "hal/interrupts/gdt.h"
#include "main/task/task.h"

namespace hal {

    thread_context create_context(uint64_t *stack, task::task &task) {
        thread_context context(0, reinterpret_cast<uint64_t>(&stack[-9]));
        // TODO: proper SS

        // New state
        stack[-17] = 0xCAFEBABECAFEBABE;						    // RAX
        stack[-16] = 0xFAFAFAFAFAFAFAFA;						    // RCX
        stack[-15] = 0xDEADBEEFDEADBEEF;						    // RDX
        stack[-14] = 0xDEADBEEFDEADBEEF;						    // R8
        stack[-13] = 0xDEADBEEFDEADBEEF;						    // R9
        stack[-12] = 0xDEADBEEFDEADBEEF;						    // R10
        stack[-11] = 0xDEADBEEFDEADBEEF;		    			    // R11
        stack[-10] = 0xDEADBEEFDEADBEEF;						    // RSI
        stack[-9] = 0xDEADBEEFDEADBEEF;							    // RDI
        stack[-8] = 0;											    // Interrupt number
        stack[-7] = 0;										        // Error code
        stack[-6] = reinterpret_cast<uint64_t>(&task::task::start);	// Return EIP
        stack[-5] = hal::glob_desc_table::instance()
                            .get_kernel_selector()
                            .get_segment_selector();			    // CS
        stack[-4] = 0;							                    // RFLAGS
        stack[-3] = reinterpret_cast<uint64_t>(&stack[-2]);		    // Return RSP
        stack[-2] = 0;							                    // Return SS
        stack[-1] = 0xDEADCAFECAAAFEEE;			                    // Start_task return EIP (must not return!)
        stack[0] = reinterpret_cast<uint64_t>(&task);			    // Start_task function parameter

        __asm__ volatile("pushfq \n\t"
                "popq %%rax\n\t"
                "movq %%rax, %0 \n\t"
                : "=m"(stack[-4])
                :
                : "%rax"
            );
        
        return context;
    }

} // namespace hal