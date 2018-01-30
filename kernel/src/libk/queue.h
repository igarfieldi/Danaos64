#ifndef DANAOS_KERNEL_LIBK_QUEUE_H_
#define DANAOS_KERNEL_LIBK_QUEUE_H_

#include <stddef.h>

namespace util {

    template < class T >
    class queue {
    public:
        static constexpr size_t MAX_SIZE = 50;
        using type = T;

    private:
        T m_data[MAX_SIZE];
        size_t head, tail;

    public:
        queue() : m_data{}, head(0), tail(0) {}
        
        void enqueue(const T &data) {
            if(size() < MAX_SIZE) {
                m_data[tail++ % MAX_SIZE] = data;
            }
        }

        T front() {
            if(is_empty()) {
                return 0;
            }
            return m_data[head];
        }

        T dequeue() {
            if(is_empty()) {
                return 0;
            }
            return m_data[head++ % MAX_SIZE];
        }

        constexpr size_t size() const {
            return tail - head;
        }

        constexpr bool is_empty() const {
            return size() == 0;
        }
    };

} // namespace util

#endif //DANAOS_KERNEL_LIBK_QUEUE_H_