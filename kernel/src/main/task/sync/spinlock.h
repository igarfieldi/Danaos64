#ifndef DANAOS_KERNEL_MAIN_TASK_SYNC_SPINLOCK_H_
#define DANAOS_KERNEL_MAIN_TASK_SYNC_SPINLOCK_H_

namespace sync {

	class spin_lock {
	private:
    	int m_locked;

	public:
		spin_lock() noexcept;
		spin_lock(bool locked) noexcept;
		~spin_lock() noexcept;

		spin_lock(const spin_lock &) = delete;
		spin_lock(spin_lock &&) = delete;
		spin_lock &operator=(const spin_lock &) = delete;
		spin_lock &operator=(spin_lock &&) = delete;

		void aquire() noexcept;
		void release() noexcept;
	};

} // namespace sync

#endif //DANAOS_KERNEL_MAIN_TASK_SYNC_SPINLOCK_H_