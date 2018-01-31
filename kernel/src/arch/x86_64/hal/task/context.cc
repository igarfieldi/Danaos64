#include "context.h"
#include "hal/interrupts/gdt.h"
#include "main/task/task.h"

namespace hal {

    thread_context create_context(uint64_t *stack, task::task &task) {
        thread_context context{reinterpret_cast<uint64_t>(&stack[-17])};

        // New state
        stack[-17] = reinterpret_cast<uint64_t>(&stack[-16]);           // RSP
        stack[-16] = 0xAAAAAAAAAAAAAAAA;                                // RAX
        stack[-15] = 0xCCCCCCCCCCCCCCCC;                                // RcX
        stack[-14] = 0xDDDDDDDDDDDDDDDD;                                // RDX
        stack[-13] = 0x8888888888888888;                                // R8
        stack[-12] = 0x9999999999999999;                                // R9
        stack[-11] = 0x1111111111111111;                                // R10
        stack[-10] = 0x2222222222222222;                                // R11
        stack[-9] = 0x0;                                                // RSI
        stack[-8] = reinterpret_cast<uint64_t>(&task);                  // RDI
        stack[-7] = 0x0;                                                // Interrupt number
        stack[-6] = 0x0;                                                // Error code
        stack[-5] = 0x0;                                                // Error code
        stack[-4] = reinterpret_cast<uint64_t>(&task::task::start);     // Return RIP
        stack[-3] = 0x8;                                                // Return CS
        stack[-2] = 0x8;                                                // RFLAGS
        stack[-1] = reinterpret_cast<uint64_t>(&stack[-16]);            // Return RSP
        stack[0] = 0x10;                                                // Return SS
        
        return context;
    }

} // namespace hal
