#include "math.h"

uint32_t powull(uint32_t base, uint32_t exp) {
    uint32_t res = 1;

    while (exp) {
        res *= base;
        exp--;
    }

    return res;
}

uint32_t logull(uint32_t base, uint32_t arg) {
    int32_t res = 0;
    uint32_t temp = 1;
    uint32_t oldTemp = 1;

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
