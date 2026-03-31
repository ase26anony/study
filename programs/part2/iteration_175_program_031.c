/* main.c - Primary driver with multiple triggering mechanisms */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Include header with inline functions for multi-TU usage */
#include "int128_helpers.h"

/* Function with nothrow attribute to influence TREE_NOTHROW flag */
void __attribute__((nothrow)) atomic_update(volatile __int128 *ptr, __int128 val) {
    __int128 expected, desired;
    
    do {
        expected = *ptr;
        desired = expected + val;
        /* Use atomic compare-exchange on __int128 - may trigger helper */
    } while (!__atomic_compare_exchange_n(ptr, &expected, desired, 
                                          0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED));
}

int main() {
    volatile __int128 volatile_var = 0;
    __int128 dividend = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 divisor = 0x10000000000000001ULL;
    __int128 result = 0;
    
    /* 1. 128-bit division - likely triggers __divti3 helper */
    result = dividend / divisor;
    
    /* 2. 128-bit modulo - likely triggers __modti3 helper */
    __int128 remainder = dividend % divisor;
    
    /* 3. Atomic operations on __int128 */
    __int128 atomic_var = 0;
    
    /* Atomic load */
    __int128 loaded = __atomic_load_n(&atomic_var, __ATOMIC_SEQ_CST);
    
    /* Atomic store */
    __atomic_store_n(&atomic_var, dividend, __ATOMIC_SEQ_CST);
    
    /* 4. Use nothrow function with volatile __int128 */
    atomic_update(&volatile_var, result);
    
    /* 5. Call inline function from header (for multi-TU usage) */
    __int128 mult_result = multiply_int128(dividend, divisor);
    
    /* 6. OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Complex operation inside target region */
        result = result * 2 - 1;
    }
    #endif
    
    /* 7. OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(result)
    {
        result = result + remainder;
    }
    #endif
    
    /* Prevent dead code elimination by printing hash */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    
    /* Simple hash combining */
    uint64_t hash = high ^ low ^ (uint64_t)mult_result;
    printf("Result hash: 0x%016llx\n", (unsigned long long)hash);
    
    /* Use all variables to prevent optimization */
    volatile_var = loaded + remainder;
    
    return (int)(hash & 0x7FFFFFFF);
}
