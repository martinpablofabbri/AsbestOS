#include "threads/fixed_point.h"

/** Fixed point with the lowest 14 bits as fractional bits **/
#define FIXED_MULTIPLIER 16384 // 2 ** 14

fixed int2fixed(int32_t n) {
    fixed ret = n * FIXED_MULTIPLIER;
    return ret;
}

int32_t fixed2intRoundTowardZero(fixed x) {
    return x / FIXED_MULTIPLIER;
}

int32_t fixed2intRoundClosest(fixed x) {
    if (x >= 0) {
	return fixed2intRoundTowardZero(x + FIXED_MULTIPLIER / 2);
    } else {
	return fixed2intRoundTowardZero(x - FIXED_MULTIPLIER / 2);
    }
}

fixed fixedAdd(fixed lhs, fixed rhs) {
    return lhs + rhs;
}

fixed fixedSubtract(fixed lhs, fixed rhs) {
    return lhs - rhs;
}

fixed fixedAddInt(fixed lhs, int32_t rhs) {
    return lhs + rhs * FIXED_MULTIPLIER;
}

fixed fixedSubtractInt(fixed lhs, int32_t rhs) {
    return lhs - rhs * FIXED_MULTIPLIER;
}

fixed fixedMultiply(fixed lhs, fixed rhs) {
    return ((int64_t) lhs) * rhs / FIXED_MULTIPLIER;
}

fixed fixedMultiplyInt(fixed lhs, int32_t rhs) {
    return lhs * rhs;
}

fixed fixedDivide(fixed lhs, fixed rhs) {
    return ((int64_t) lhs) * FIXED_MULTIPLIER / rhs;
}

fixed fixedDivideInt(fixed lhs, int32_t rhs) {
    return lhs / rhs;
}
