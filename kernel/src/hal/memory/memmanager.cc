#include "memmanager.h"

namespace hal {

    memory_manager::memory_manager() {
    }

    memory_manager &memory_manager::instance() {
        static memory_manager inst;
        return inst;
    }
}