#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Header file to be included in multiple translation units */
#include "triggers.h"

/* Function with nothrow attribute */
void __attribute__((nothrow)) atomic_update(volatile __int128 *ptr, __int128 val) {
    __int128 expected, desired;
    do {
        expected = __atomic_load_n(ptr, __ATOMIC_RELAXED);
        desired = expected + val;
    } while (!__atomic_compare_exchange_n(ptr, &expected, desired, 
                                          0, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
}

int main() {
    volatile __int128 volatile_var = 100;
    __int128 dividend = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 divisor = 0x1000000000000000ULL;
    __int128 result = 0;
    
    /* Trigger 128-bit division helper */
    for (int i = 0; i < 10; i++) {
        result += dividend / (divisor + i);
        result += dividend % (divisor + i);
    }
    
    /* Trigger atomic helper */
    __int128 atomic_var = 0;
    __atomic_store_n(&atomic_var, result, __ATOMIC_RELAXED);
    
    /* Use volatile variable */
    result += volatile_var;
    
    /* Call nothrow function with atomic operation */
    atomic_update(&atomic_var, result);
    
    /* OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        result = result * 2 - 1;
    }
    #endif
    
    /* OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(result)
    {
        result = result / 3;
    }
    #endif
    
    /* Print hash to prevent optimization */
    unsigned long long high = (unsigned long long)(result >> 64);
    unsigned long long low = (unsigned long long)result;
    printf("Result hash: %016llx%016llx\n", high, low);
    
    /* Call inline function from header */
    result = process_int128(result);
    
    return 0;
}
