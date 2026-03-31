#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function in header to force multiple translation unit usage */
static inline __int128 calculate_hash(__int128 a, __int128 b) {
    /* Complex 128-bit operation that may require helper functions */
    volatile __int128 volatile_val = a;
    __int128 result = volatile_val / b;  /* May trigger __divti3 helper */
    
    /* Atomic operation on 128-bit value */
    __int128 atomic_val = 0;
    __atomic_store(&atomic_val, &result, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Function with nothrow attribute */
__attribute__((nothrow)) 
static void process_128bit(__int128 *dest, __int128 src) {
    /* Another 128-bit operation */
    *dest = src % (*dest + 1);  /* May trigger __umodti3 helper */
}

int main() {
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x1000000000000000ULL;
    __int128 result = 0;
    
    /* Loop with 128-bit division */
    for (int i = 0; i < 10; i++) {
        result += calculate_hash(a + i, b + i);
    }
    
    /* Process with nothrow function */
    process_128bit(&result, a);
    
    /* OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Simple operation in target region */
        result = result * 2;
    }
    #endif
    
    /* OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(result)
    {
        result = result + 1;
    }
    #endif
    
    /* Atomic compare exchange on 128-bit */
    __int128 expected = result;
    __int128 desired = result * 3;
    __atomic_compare_exchange(&result, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Prevent dead code elimination - print hash */
    unsigned long long hi = (unsigned long long)(result >> 64);
    unsigned long long lo = (unsigned long long)result;
    printf("Result hash: 0x%016llx%016llx\n", hi, lo);
    
    return 0;
}
