#include "gdt.h"
#include "hal/memory/memmanager.h"
#include "debug/trace.h"
#include "main/kernel.h"

namespace hal {

	glob_desc_table::glob_desc_table() : m_capacity(0), m_index(0), m_entries(nullptr), m_descriptor() {
	}

	glob_desc_table::~glob_desc_table() noexcept {
		// TODO: release resources
	}

	glob_desc_table &glob_desc_table::instance() noexcept {
		static glob_desc_table instance;
		return instance;
	}
	
	void glob_desc_table::init() noexcept {
		if(m_entries != nullptr) {
			kernel::panic("GDT cannot be initialized more than once!");
		}

		this->m_capacity = 5;
		m_entries = memory_manager::instance().kernel_malloc<entry>(m_capacity);
		m_descriptor = descriptor{0, reinterpret_cast<uint64_t>(m_entries)};

		// Set up the descriptor entries
		// 1. Null descriptor
		this->push_entry(entry(0));
		// 2. Kernel code segment
		this->push_entry(entry(0, 0xFFFFFFFF, dpl_level::RING_0, true, true, false, true, true, true));
		// 3. Kernel data segment
		this->push_entry(entry(0, 0xFFFFFFFF, dpl_level::RING_0, true, true, false, false, true, true));
		// 2. User code segment
		this->push_entry(entry(0, 0xFFFFFFFF, dpl_level::RING_3, true, true, false, true, true, true));
		// 3. User data segment
		this->push_entry(entry(0, 0xFFFFFFFF, dpl_level::RING_3, true, true, false, false, true, true));

		// Reload the descriptor
		this->load();
	}

	uint16_t glob_desc_table::push_entry(entry entry) noexcept {
		if(m_index < m_capacity) {
			m_entries[m_index] = entry;
			return m_index++;
		} else {
			return INVALID_INDEX;
		}
	}

	void glob_desc_table::set_entry(entry entry, uint16_t index) noexcept {
		if((index < m_index) && (index != 0)) {
			m_entries[index] = entry;
		} else {
			kernel::panic("Invalid GDT index!");
		}
	}

	glob_desc_table::entry &glob_desc_table::get_entry(uint16_t index) const noexcept {
		if(index >= m_capacity) {
			kernel::panic("Invalid GDT index!");
		}

		return m_entries[index];
	}

	glob_desc_table::selector glob_desc_table::get_selector(uint16_t index) const noexcept {
		if(index >= m_capacity) {
			kernel::panic("Invalid GDT index!");
		}
		return selector(m_entries[index].get_privilege_level(), false, index);
	}

	glob_desc_table::selector glob_desc_table::get_kernel_selector() const noexcept {
		return selector(m_entries[KERNEL_CODE_INDEX].get_privilege_level(), false, KERNEL_CODE_INDEX);
	}
	
	void glob_desc_table::load() noexcept {
		m_descriptor.size = static_cast<uint16_t>(sizeof(entry) * m_index - 1);
		__asm__ volatile("lgdt (%0)" : : "d"(reinterpret_cast<uint64_t>(&m_descriptor)));

		kernel::m_console.print("  GDT off      : []\n", m_descriptor.offset);
		kernel::m_console.print("  GDT size     : {}\n", m_descriptor.size);
	}

} // namespace hal
