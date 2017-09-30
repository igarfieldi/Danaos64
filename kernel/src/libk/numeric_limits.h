#ifndef DANAOS_KERNEL_LIBK_LIMITS_
#define DANAOS_KERNEL_LIBK_LIMITS_

#include <stdint.h>
#include <limits.h>

// For w.e reason these may not be defined
#ifndef LLONG_MAX
    #define LLONG_MAX   9223372036854775807LL
#endif //LLONG_MAX
#ifndef LLONG_MIN
    #define LLONG_MIN   (-LLONG_MAX - 1)
#endif //LLONG_MIN
#ifndef ULLONG_MAX
    #define ULLONG_MAX  18446744073709551615ULL
#endif //ULLONG_MAX

namespace std {

    // Very rudimentary implementation (lacks all but min/max)!

    template < class T >
    struct numeric_limits {
        static constexpr bool is_specialized = false;
    };
    
    template <>
    struct numeric_limits<char> {   
        static constexpr bool is_specialized = true;

        static constexpr char min() {
            return CHAR_MIN;
        }

        static constexpr char max() {
            return CHAR_MAX;
        }
    };
    
    template <>
    struct numeric_limits<unsigned char> {   
        static constexpr bool is_specialized = true;

        static constexpr unsigned char min() {
            return 0;
        }

        static constexpr unsigned char max() {
            return UCHAR_MAX;
        }
    };
    
    template <>
    struct numeric_limits<int> {   
        static constexpr bool is_specialized = true;

        static constexpr int min() {
            return INT_MIN;
        }

        static constexpr int max() {
            return INT_MAX;
        }
    };
    
    template <>
    struct numeric_limits<unsigned int> {
        static constexpr bool is_specialized = true;

        static constexpr unsigned int min() {
            return 0;
        }

        static constexpr unsigned int max() {
            return UINT_MAX;
        }
    };

    template <>
    struct numeric_limits<long> {
        static constexpr bool is_specialized = true;

        static constexpr long min() {
            return LONG_MIN;
        }

        static constexpr long max() {
            return LONG_MAX;
        }
    };
    
    template <>
    struct numeric_limits<unsigned long> {
        static constexpr bool is_specialized = true;

        static constexpr unsigned long min() {
            return 0;
        }

        static constexpr unsigned long max() {
            return ULONG_MAX;
        }
    };

    template <>
    struct numeric_limits<long long> {
        static constexpr bool is_specialized = true;

        static constexpr long long min() {
            return LLONG_MIN;
        }

        static constexpr long long max() {
            return LLONG_MAX;
        }
    };

    template <>
    struct numeric_limits<unsigned long long> {
        static constexpr bool is_specialized = true;

        static constexpr unsigned long long min() {
            return 0;
        }

        static constexpr unsigned long long max() {
            return ULLONG_MAX;
        }
    };
} // namespace std

#endif //DANAOS_KERNEL_LIBK_LIMITS_