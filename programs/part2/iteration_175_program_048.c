/* Test program to trigger target hook for generating helper functions */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Inline function that uses 128-bit operations - will be included in header */
static inline __int128 complex_128bit_operation(__int128 a, __int128 b) {
    /* Complex 128-bit operation that likely requires helper functions */
    __int128 result = a / b;  /* This may call __divti3 helper */
    result += a % b;          /* This may call __modti3 helper */
    return result;
}

/* Function with nothrow attribute */
__attribute__((nothrow))
static void atomic_128bit_update(__int128 *ptr, __int128 val) {
    /* Atomic operation on 128-bit variable */
    __int128 expected = *ptr;
    __int128 desired;
    do {
        desired = expected + val;
    } while (!__atomic_compare_exchange_n(ptr, &expected, desired, 
                                          0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}

/* OpenMP target region using 128-bit variables */
#ifdef _OPENMP
void omp_128bit_test(__int128 *result) {
    #pragma omp target map(tofrom: result[0:1])
    {
        __int128 local = *result;
        /* Force some computation that might need helpers */
        if (local != 0) {
            local = local / 2;  /* Potential helper call */
        }
        *result = local;
    }
}
#endif

int main() {
    volatile __int128 volatile_var = 100;
    __int128 a = ((__int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210;
    __int128 b = ((__int128)0x1111111111111111 << 64) | 0x1111111111111111;
    __int128 result = 0;
    
    /* Loop with 128-bit division - may trigger helper generation */
    for (int i = 0; i < 10; i++) {
        result += complex_128bit_operation(a + i, b + i);
        volatile_var += 1;  /* Use volatile variable */
    }
    
    /* Atomic operation on 128-bit */
    __int128 atomic_var = 0;
    atomic_128bit_update(&atomic_var, result);
    
    /* OpenMP target region if available */
    #ifdef _OPENMP
    omp_128bit_test(&atomic_var);
    #endif
    
    /* OpenACC region if available */
    #ifdef _OPENACC
    #pragma acc parallel copy(atomic_var)
    {
        #pragma acc loop reduction(+:atomic_var)
        for (int i = 0; i < 100; i++) {
            atomic_var += i;
        }
    }
    #endif
    
    /* Prevent dead code elimination */
    uint64_t high = (uint64_t)(atomic_var >> 64);
    uint64_t low = (uint64_t)atomic_var;
    printf("Result hash: 0x%016lx%016lx\n", high, low);
    
    return 0;
}
