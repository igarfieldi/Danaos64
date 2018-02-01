#include "scheduler.h"
#include "main/kernel.h"

namespace task {

    scheduler::scheduler() noexcept : m_tasks(), m_idle(&scheduler::idle, 0 , nullptr), m_active(&m_idle) {}

    scheduler &scheduler::instance() noexcept {
        static scheduler instance;
        return instance;
    }
    
    void scheduler::idle(int argc, char ** argv) noexcept {
        (void) argc;
        (void) argv;

        kernel::m_console.print("Idling\n");
        while(true) {
            asm volatile("sti\t\nhlt\t\n");
        }
    }

    void scheduler::start() noexcept {
        if(!m_tasks.is_empty()) {
            m_active = m_tasks.dequeue();
        }
        m_active->ready();
        task::start(*m_active);
    }

    void scheduler::ready(task &task) noexcept {
        m_tasks.enqueue(&task);
        task.ready();
    }

    void scheduler::block() noexcept {
        // Block the current task and yield to the next queue'd one
        m_tasks.front()->block();
        yield();
    }

    task &scheduler::active_task() noexcept {
        return *m_active;
    }

    void scheduler::yield() noexcept {
        // If no other task is active don't do anything
        if(m_tasks.is_empty()) {
            if(m_active->is_finished()) {
                task &curr = *m_active;
                m_active = &m_idle;
                curr.task_switch(*m_active);
            }
        } else {
            task &curr = *m_active;
            task *next = m_tasks.dequeue();

            // Loop the tasks until one is found that is ready
            while(!next->is_ready()) {
                if(!next->is_finished()) {
                    m_tasks.enqueue(next);
                }
                next = m_tasks.dequeue();
            }

            // Re-queue the current task and continue the next one
                // We don't requeue the idle task either
            if(!curr.is_finished() && (&curr != &m_idle)) {
            	m_tasks.enqueue(&curr);
            }
            m_active = next;

            return curr.task_switch(*next);
        }
    }

    const hal::task_context &scheduler::schedule(const hal::task_context &curr_context) noexcept {
        // If no other task is active don't do anything
        if(m_tasks.is_empty()) {
            if(m_active->is_finished()) {
                task &curr = *m_active;
                m_active = &m_idle;
                return curr.task_switch(curr_context, *m_active);
            } else {
                return curr_context;
            }
        } else {
            task &curr = *m_active;
            task *next = m_tasks.dequeue();

            // Loop the tasks until one is found that is ready
            while(!next->is_ready()) {
                if(!next->is_finished()) {
                    m_tasks.enqueue(next);
                }
                next = m_tasks.dequeue();
            }

            // Re-queue the current task and continue the next one
                // We don't requeue the idle task either
            if(!curr.is_finished() && (&curr != &m_idle)) {
            	m_tasks.enqueue(&curr);
            }
            m_active = next;

            return curr.task_switch(curr_context, *next);
        }
    }
}
