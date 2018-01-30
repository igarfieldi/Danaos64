#include "scheduler.h"
#include "main/kernel.h"

namespace task {

    scheduler::scheduler() : m_tasks(), m_active(nullptr) {}

    scheduler &scheduler::instance() {
        static scheduler instance;
        return instance;
    }

    void scheduler::start(void (*func)(void)) {
        // This will live as long as the task isn't ending, at which point it won't matter
        task init(func);
        m_active = &init;
        init.ready();
        task::start(init);
    }

    void scheduler::ready(task &task) {
        m_tasks.enqueue(&task);
        task.ready();
    }

    void scheduler::block() {
        // Block the current task and yield to the next queue'd one
        m_tasks.front()->block();
        yield();
    }

    void scheduler::yield() const {
        __asm__ volatile("int $0" : :);
    }

    const hal::thread_context &scheduler::schedule(const hal::thread_context &curr_context) {
        // If no other task is active don't do anything
        if(m_tasks.size() == 0) {
            return curr_context;
        } else {
            task *curr = m_active;
            task *next = m_tasks.dequeue();

            // Loop the tasks until one is found that is ready
            while(!next->is_ready()) {
                if(!next->is_finished()) {
                    m_tasks.enqueue(next);
                }
                next = m_tasks.dequeue();
            }

            // Re-queue the current task and continue the next one
            m_tasks.enqueue(curr);
            m_active = next;

            return curr->task_switch(curr_context, *next);
        }
    }
}