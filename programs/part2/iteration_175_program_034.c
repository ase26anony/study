/* Test program to trigger target hook for generating helper declarations */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Inline function that uses 128-bit operations - will be included in header */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This division may require runtime helper on targets without native 128-bit support */
    return a / b;
}

/* Function with nothrow attribute */
int __attribute__((nothrow)) atomic_update(__int128 *ptr, __int128 val) {
    __int128 expected = *ptr;
    __int128 desired = expected + val;
    
    /* Use atomic compare-exchange on 128-bit variable */
    /* This often requires helper functions */
    return __atomic_compare_exchange_n(ptr, &expected, desired, 
                                       0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Volatile 128-bit variable */
volatile __int128 volatile_128 = 0;

int main(void) {
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 100;
    __int128 result = 0;
    
    /* Perform 128-bit division - may trigger helper generation */
    result = do_128bit_division(a, b);
    
    /* Use volatile 128-bit variable */
    volatile_128 = result;
    
    /* Atomic operation on 128-bit variable */
    __int128 atomic_var = 0;
    __atomic_store_n(&atomic_var, result, __ATOMIC_RELAXED);
    
    /* OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: atomic_var) if(0)
    {
        /* Simple operation inside target region */
        atomic_var = atomic_var + 1;
    }
    #endif
    
    /* Call nothrow function with atomic operation */
    atomic_update(&atomic_var, 100);
    
    /* Print result to prevent optimization */
    uint64_t hi = (uint64_t)(atomic_var >> 64);
    uint64_t lo = (uint64_t)atomic_var;
    printf("Result hash: 0x%016llx%016llx\n", (unsigned long long)hi, (unsigned long long)lo);
    
    return 0;
}
