#include "math.h"

namespace math {

    long long powl(long long base, unsigned long long exp) {
        long res = 1;

        while (exp) {
            res *= base;
            exp--;
        }

        return res;
    }

    unsigned long long logl(unsigned long long base, unsigned long long arg) {
        long long res = 0;
        unsigned long long temp = 1;
        unsigned long long oldTemp = 1;

        while ((temp *= base) <= arg) {
            if (oldTemp > temp) // overflow happened (TODO: this might not always work!)
                return res;
            res++;
        }

        return res;
    }

} // namespace math