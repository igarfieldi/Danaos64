#ifndef DANAOS_KERNEL_MAIN_TASK_SCHEDULER_H_
#define DANAOS_KERNEL_MAIN_TASK_SCHEDULER_H_

#include "main/task/task.h"
#include "libk/queue.h"
#include "hal/task/context.h"


namespace task {

    class scheduler {
    private:
        util::queue<task*> m_tasks;
        task m_idle;
        task *m_active;

        static void idle(int argc, char ** argv) noexcept;

    public:
        scheduler() noexcept;

        static scheduler &instance() noexcept;
        void start() noexcept;
        void ready(task &task) noexcept;
        void block() noexcept;

        void yield() noexcept;
        const hal::task_context &schedule(const hal::task_context &curr_context) noexcept;
        task &active_task() noexcept;
    };

} // namespace task

#endif //DANAOS_KERNEL_MAIN_TASK_SCHEDULER_H_
