#include "spinlock.h"

extern void _aquire_spinlock(int *lock) __asm__("_aquire_spinlock");
extern void _release_spinlock(int *lock) __asm__("_release_spinlock");

namespace sync {

	spin_lock::spin_lock() noexcept : m_locked(false) {}
	spin_lock::spin_lock(bool locked) noexcept : m_locked(locked) {}

	spin_lock::~spin_lock() noexcept {
		// TODO: should this really happen?
		this->release();
	}

	void spin_lock::aquire() noexcept {
		_aquire_spinlock(&this->m_locked);
	}

	void spin_lock::release() noexcept {
		_release_spinlock(&this->m_locked);
	}

} // namespace sync