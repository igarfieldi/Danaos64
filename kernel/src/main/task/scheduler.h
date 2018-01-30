#ifndef DANAOS_KERNEL_MAIN_TASK_SCHEDULER_H_
#define DANAOS_KERNEL_MAIN_TASK_SCHEDULER_H_

#include "main/task/task.h"
#include "libk/queue.h"
#include "hal/task/context.h"


namespace task {

    class scheduler {
    private:
        util::queue<task*> m_tasks;
        task *m_active;

    public:
        scheduler();

        static scheduler &instance();
        void start(void (*func)(void));
        void ready(task &task);
        void yield() const;
        void block();

        const hal::thread_context &schedule(const hal::thread_context &curr_context);
    };

} // namespace task

#endif //DANAOS_KERNEL_MAIN_TASK_SCHEDULER_H_