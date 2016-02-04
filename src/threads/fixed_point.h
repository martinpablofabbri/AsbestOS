/*! \file fixed_point.h
 *
 * Declarations for the fixed point arithmetic implementation
 */

#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#include <stdint.h>

/*! Fixed point float type. */
typedef int32_t fixed;

/*! Conversions between fixed and int */
fixed int2fixed(int32_t n);
int32_t fixed2intRoundTowardZero(fixed x);
int32_t fixed2intRoundClosest(fixed x);

/*! Fixed-point operators */
fixed fixedAdd(fixed lhs, fixed rhs);
fixed fixedSubtract(fixed lhs, fixed rhs);
fixed fixedAddInt(fixed lhs, int32_t rhs);
fixed fixedSubtractInt(fixed lhs, int32_t rhs);
fixed fixedMultiply(fixed lhs, fixed rhs);
fixed fixedMultiplyInt(fixed lhs, int32_t rhs);
fixed fixedDivide(fixed lhs, fixed rhs);
fixed fixedDivideInt(fixed lhs, int32_t rhs);

#endif /* threads/fixed_point.h */

