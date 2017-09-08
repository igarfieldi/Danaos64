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

    long powl(long base, unsigned long exp);
    unsigned long logl(unsigned long base, unsigned long arg);

} // namespace math

#endif //DANAOS_LIBK_MATH_H_