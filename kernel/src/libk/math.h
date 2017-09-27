#ifndef DANAOS_LIBK_MATH_H_
#define DANAOS_LIBK_MATH_H_

namespace math {

    template < class T >
    T min(T a, T b) {
        return (a < b) ? a : b;
    }

    template < class T >
    T max(T a, T b) {
        return (a > b) ? a : b;
    }

    long long powl(long long base, unsigned long long exp);
    unsigned long long logl(unsigned long long base, unsigned long long arg);

} // namespace math

#endif //DANAOS_LIBK_MATH_H_