#include "context.h"
#include "hal/interrupts/gdt.h"
#include "main/task/task.h"

namespace hal {

    task_context create_context(volatile uint64_t *stack, task::task &task) {
        task_context context(&stack[-21]);

        // New state
        stack[-21] = reinterpret_cast<uint64_t>(&stack[0]);             // RBP
        stack[-20] = 0xAAAAAAAAAAAAAAAA;                                // RAX
        stack[-19] = 0xBBBBBBBBBBBBBBBB;                                // RBX
        stack[-18] = 0xCCCCCCCCCCCCCCCC;                                // RCX
        stack[-17] = 0xDDDDDDDDDDDDDDDD;                                // RDX
        stack[-16] = 0x8888888888888888;                                // R8
        stack[-15] = 0x9999999999999999;                                // R9
        stack[-14] = 0x0000000000000000;                                // R10
        stack[-13] = 0x1111111111111111;                                // R11
        stack[-12] = 0x2222222222222222;                                // R12
        stack[-11] = 0x3333333333333333;                                // R13
        stack[-10] = 0x4444444444444444;                                // R14
        stack[-9] = 0x5555555555555555;                                 // R15
        stack[-8] = 0x5151515151515151;                                 // RSI
        stack[-7] = reinterpret_cast<uint64_t>(&task);                  // RDI
        stack[-6] = 0x0;                                                // Interrupt number
        stack[-5] = 0x0;                                                // Error code
        stack[-4] = reinterpret_cast<uint64_t>(&task::task::start);     // Return RIP
        stack[-3] = 0x8;                                                // Return CS
        stack[-2] = 0x200;                                              // RFLAGS (enable interrupts)
        stack[-1] = reinterpret_cast<uint64_t>(&stack[0]);              // Return RSP
        stack[0] = 0x10;                                                // Return SS
        
        return context;
    }

} // namespace hal
