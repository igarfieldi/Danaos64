#include "memmanager.h"

namespace hal {

    memory_manager::memory_manager() : m_page_frame_count(0) {
    }

    memory_manager &memory_manager::instance() {
        static memory_manager inst;
        return inst;
    }
}