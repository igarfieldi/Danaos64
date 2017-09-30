#include "math.h"

namespace math {

    unsigned long long powull(unsigned long long base, unsigned long long exp) {
        unsigned long long res = 1;

        while (exp) {
            res *= base;
            exp--;
        }

        return res;
    }

    unsigned long long logull(unsigned long long base, unsigned long long arg) {
        long long res = 0;
        unsigned long long temp = 1;
        unsigned long long oldTemp = 1;

        while ((temp *= base) <= arg) {
            // Check for overflow!
            if(oldTemp > temp) {
                return res;
            }
            res++;
            oldTemp = temp;
        }

        return res;
    }

} // namespace math