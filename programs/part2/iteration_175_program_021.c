#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Inline function that uses 128-bit operations - will be included in header */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This division may require runtime helper on targets without native 128-bit support */
    return a / b;
}

/* Function with nothrow attribute */
__attribute__((nothrow))
static void atomic_128bit_op(__int128 *val) {
    __int128 desired = 100;
    __int128 expected = *val;
    
    /* Atomic compare-exchange on 128-bit - may require helper */
    __atomic_compare_exchange_n(val, &expected, desired, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Volatile 128-bit variable */
volatile __int128 volatile_128 = 0;

int main() {
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 100;
    __int128 result = 0;
    
    /* Use volatile 128-bit variable */
    volatile_128 = a;
    
    /* Perform 128-bit division - may trigger helper generation */
    result = do_128bit_division(a, b);
    
    /* Atomic operation on 128-bit */
    __int128 atomic_val = 50;
    atomic_128bit_op(&atomic_val);
    
    /* OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Simple operation in target region */
        result = result + 1;
    }
    #endif
    
    /* OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(result)
    {
        result = result * 2;
    }
    #endif
    
    /* Print result to prevent optimization */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    printf("Result: 0x%016llx%016llx\n", (unsigned long long)high, 
                                         (unsigned long long)low);
    
    return 0;
}
