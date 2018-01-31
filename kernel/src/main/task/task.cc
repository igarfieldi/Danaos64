#include "task.h"
#include "main/task/scheduler.h"
#include "main/kernel.h"

namespace task {

    task::task(void (*start)()) : m_context(hal::create_context(&stack[STACK_SIZE-1], *this)), m_state(state::CREATED), m_func(start) {
    }

    void task::start(task &task) {
        // Start the task by calling the stored function
        kernel::m_console.print("Task begins!\n");
        task.m_func();
        kernel::m_console.print("Task is over!\n");

        // Give task free for scheduler cleanup and yield to next task
        task.m_state = state::FINISHED;
        scheduler::instance().yield();
    }

    void task::block() {
        m_state = state::BLOCKED;
    }

    void task::ready() {
        m_state = state::READY;
    }

    void task::task_switch(task &next) {
        this->m_state = state::WAITING;
        next.m_state = state::RUNNING;

        // Swap the state of the tasks and exchange the contexts
        switch_context(&m_context, &next.m_context);
    }

    const hal::thread_context &task::task_switch(const hal::thread_context &context, task &next) {
        this->m_state = state::WAITING;
        next.m_state = state::RUNNING;

        // Swap the state of the tasks and exchange the contexts
        m_context = context;
        return next.m_context;
    }

} // namespace task