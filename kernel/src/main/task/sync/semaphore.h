#ifndef DANAOS_KERNEL_MAIN_TASK_SYNC_SEMAPHORE_H_
#define DANAOS_KERNEL_MAIN_TASK_SYNC_SEMAPHORE_H_

#include "libk/queue.h"
#include "main/task/task.h"

namespace sync {

	class semaphore {
	private:
		size_t m_count;		// TODO: needs to be atomic!
		util::queue<task::task*> m_queue;

	public:
		semaphore();
		
		void wait();
		void signal();
	};

} // namespace sync

#endif //DANAOS_KERNEL_MAIN_TASK_SYNC_SEMAPHORE_H_