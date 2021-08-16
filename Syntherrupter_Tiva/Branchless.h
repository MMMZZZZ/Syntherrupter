/*
 * Branchless.h
 *
 *  Created on: 16.08.2021
 *      Author: Max Zuidberg
 *
 *
 */

#ifndef BRANCHLESS_H_
#define BRANCHLESS_H_


#include <stdbool.h>
#include <stdint.h>


namespace Branchless {

template<typename T>
inline T selectByCond(T a, T b, bool c)
{
    /*
     * Equivalent of
     * if (c)
     *  return a;
     * else
     *  return b;
     */
    return c * a + (!c) * b;
}

template<typename T>
inline T max(T a, T b)
{
    /*
     * Branchless version of std::max(a, b)
     * Speeds (with -O3):
     *   std::max:   110-140ns
     *   ? operator: 100ns
     *   branchless:  60ns
     *
     * Notes:
     *  * with no optimizations, this is much slower because the function won't
     *    be inlined.
     *  * one reason for this being so fast is that the M4F has a multiply-
     *    accumulate instruction. That makes my solution faster than f.ex.
     *    this one (no profiling done, but a couple less instructions):
     *    https://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
     *  * Profiling has only been done for ui32. Should work for floats, too
     *    but needs to be tested. At least one extra bool->float conversion
     *    is required.
     *
     */
    return selectByCond(a, b, (a > b));
}

template<typename T>
inline T min(T a, T b)
{
    return selectByCond(a, b, (a < b));
}

template<typename T>
inline T abs(T a)
{
    return selectByCond(a, -a, (a >= 0));
}

template<typename T>
inline T sgn(T a)
{
    return selectByCond(1, -1, (a >= 0));
}

};

#endif /* BRANCHLESS_H_ */
