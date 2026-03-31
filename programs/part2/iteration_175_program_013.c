#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Header file to be included in multiple translation units */
#include "triggers.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Function with nothrow attribute */
void nothrow_operation(volatile __int128 *dest, __int128 val) 
    __attribute__((nothrow));

#ifdef __cplusplus
}
#endif

void nothrow_operation(volatile __int128 *dest, __int128 val) {
    /* Atomic operation on volatile 128-bit variable */
    __int128 expected = *dest;
    __int128 desired = val;
    
    /* This may trigger atomic helper generation */
    __atomic_compare_exchange(dest, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

int main() {
    volatile __int128 volatile_var = 0;
    __int128 dividend = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 divisor = 0x10000000000000001ULL;
    __int128 result = 0;
    
    /* 1. 128-bit division - may generate __divti3 or similar helper */
    result = perform_128bit_division(dividend, divisor);
    
    /* 2. Atomic operation on volatile 128-bit variable */
    nothrow_operation(&volatile_var, result);
    
    /* 3. Another 128-bit operation using inline function from header */
    __int128 product = multiply_128bit(result, divisor);
    
    /* 4. OpenMP target region if supported */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: product)
    {
        /* Simple operation in target region */
        product = product + 1;
    }
    #endif
    
    /* 5. Atomic load of 128-bit value */
    __int128 loaded;
    __atomic_load(&product, &loaded, __ATOMIC_SEQ_CST);
    
    /* 6. 128-bit modulo operation - may generate __umodti3 */
    unsigned __int128 udividend = (unsigned __int128)dividend;
    unsigned __int128 udivisor = (unsigned __int128)divisor;
    unsigned __int128 umod = udividend % udivisor;
    
    /* Combine results to prevent optimization */
    result = result + (__int128)umod + loaded;
    
    /* Print hash to prevent dead code elimination */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    printf("Result hash: 0x%016llx%016llx\n", 
           (unsigned long long)high, 
           (unsigned long long)low);
    
    return 0;
}
