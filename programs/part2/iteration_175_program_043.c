#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Inline function in header style to potentially generate helpers in multiple TUs */
static inline __int128 complex_128bit_op(__int128 a, __int128 b) {
    /* Complex 128-bit operation that may require helper */
    return (a * a) / (b + 1);
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
__attribute__((nothrow))
static void atomic_128bit_update(volatile __int128 *ptr, __int128 val) {
    __int128 expected = *ptr;
    __int128 desired;
    do {
        desired = expected + val;
    } while (!__atomic_compare_exchange(ptr, &expected, &desired, 
                                        0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}

int main() {
    volatile __int128 volatile_var = 100;
    __int128 a = ((__int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210;
    __int128 b = ((__int128)0x1111111111111111 << 64) | 0x2222222222222222;
    __int128 result = 0;
    
    /* Loop with 128-bit division - may generate __divti3 helper */
    for (int i = 0; i < 10; i++) {
        result += a / (b + i);
        result += complex_128bit_op(a, b);
    }
    
    /* Atomic operation on 128-bit - may generate atomic helper */
    __int128 atomic_var = 0;
    __atomic_store(&atomic_var, &result, __ATOMIC_RELAXED);
    atomic_128bit_update(&atomic_var, result);
    
    /* OpenMP target region with 128-bit variable */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: atomic_var) if(0)  /* if(0) to avoid actual offloading but keep hook calls */
    {
        atomic_var = atomic_var / 2;
    }
    #endif
    
    /* OpenACC parallel region */
    #ifdef _OPENACC
    #pragma acc parallel copy(atomic_var) if(0)  /* if(0) to avoid actual offloading */
    {
        atomic_var = atomic_var + 1;
    }
    #endif
    
    /* Prevent dead code elimination */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    printf("Result hash: %016lx%016lx\n", high, low);
    
    return 0;
}
