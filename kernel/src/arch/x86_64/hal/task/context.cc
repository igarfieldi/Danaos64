#include "context.h"
#include "hal/interrupts/gdt.h"
#include "main/task/task.h"

namespace hal {

    task_context create_context(volatile uint64_t *stack, task::task &task) {
        task_context context(&stack[-16]);

        // New state
        stack[-16] = reinterpret_cast<uint64_t>(&stack[0]);             // RBP
        stack[-15] = 0xAAAAAAAAAAAAAAAA;                                // RAX
        stack[-14] = 0xCCCCCCCCCCCCCCCC;                                // RCX
        stack[-13] = 0xDDDDDDDDDDDDDDDD;                                // RDX
        stack[-12] = 0x8888888888888888;                                // R8
        stack[-11] = 0x9999999999999999;                                // R9
        stack[-10] = 0x1111111111111111;                                // R10
        stack[-9] = 0x2222222222222222;                                 // R11
        stack[-8] = 0x5151515151515151;                                 // RSI
        stack[-7] = reinterpret_cast<uint64_t>(&task);                  // RDI
        stack[-6] = 0x0;                                                // Interrupt number
        stack[-5] = 0x0;                                                // Error code
        stack[-4] = reinterpret_cast<uint64_t>(&task::task::start);     // Return RIP
        stack[-3] = 0x8;                                                // Return CS
        stack[-2] = 0;                                                  // RFLAGS
        stack[-1] = reinterpret_cast<uint64_t>(&stack[0]);              // Return RSP
        stack[0] = 0x10;                                                // Return SS
        
        return context;
    }

} // namespace hal
