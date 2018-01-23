#ifndef DANAOS_MAIN_TASK_TASK_H_
#define DANAOS_MAIN_TASK_TASK_H_

#include <stdint.h>
#include "hal/task/context.h"

namespace task {

    class task {
    public:
        enum class state {
            CREATED,
            READY,
            RUNNING,
            WAITING,
            BLOCKED,
            FINISHED
        };

    private:
        static constexpr size_t STACK_SIZE = 1024;
        volatile uintptr_t stack[STACK_SIZE];
        hal::task_context m_context;
        state m_state;
        void (*m_func)();

    public:
        task(void (*start)());
        static void start(task &task);

        constexpr bool is_ready() const {
            return (m_state == state::READY) || (m_state == state::WAITING);
        }

        constexpr bool is_blocked() const {
            return m_state == state::BLOCKED;
        }

        constexpr bool is_finished() const {
            return m_state == state::FINISHED;
        }

        void block();
        void ready();

        void task_switch(task &next);
        const hal::task_context &task_switch(const hal::task_context &context, task &next);
    };

}

#endif //DANAOS_MAIN_TASK_TASK_H_
