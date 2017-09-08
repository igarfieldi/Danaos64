#include "math.h"

namespace math {

    long powl(long base, unsigned long exp) {
        long res = 1;

        while (exp) {
            res *= base;
            exp--;
        }

        return res;
    }

    unsigned long logl(unsigned long base, unsigned long arg) {
        long res                   = 0;
        unsigned long long temp    = 1;
        unsigned long long oldTemp = 1;

        while ((temp *= base) <= arg) {
            if (oldTemp > temp) // overflow happened (TODO: this might not always work!)
                return res;
            res++;
        }

        return res;
    }

} // namespace math