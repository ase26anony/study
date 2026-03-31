#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function in header to force helper generation in multiple TUs */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* Complex 128-bit division that likely requires helper */
    return a / b;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
__attribute__((nothrow)) 
static void atomic_128bit_op(__int128 *ptr, __int128 val) {
    __int128 expected, desired;
    
    /* Atomic operations on 128-bit that may need helpers */
    expected = *ptr;
    desired = expected + val;
    
    /* Atomic compare-exchange - may generate helper calls */
    __atomic_compare_exchange(ptr, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

int main() {
    volatile __int128 volatile_var = 100;
    __int128 dividend = ((__int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210;
    __int128 divisor = 0x1000000000000000;
    __int128 result = 0;
    
    /* Loop with 128-bit division - may trigger helper generation */
    for (int i = 0; i < 10; i++) {
        result += do_128bit_division(dividend + i, divisor);
        volatile_var += i;  /* Use volatile variable */
    }
    
    /* Atomic operation on 128-bit */
    __int128 atomic_var = 0;
    atomic_128bit_op(&atomic_var, result);
    
    /* OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: atomic_var) if(0)
    {
        /* Simple operation in target region */
        atomic_var = atomic_var * 2;
    }
    #endif
    
    /* OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(atomic_var)
    {
        atomic_var = atomic_var + 1;
    }
    #endif
    
    /* Print result to prevent optimization */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    printf("Result: 0x%016llx%016llx\n", (unsigned long long)high, 
           (unsigned long long)low);
    
    /* Also print atomic variable */
    high = (uint64_t)(atomic_var >> 64);
    low = (uint64_t)atomic_var;
    printf("Atomic: 0x%016llx%016llx\n", (unsigned long long)high, 
           (unsigned long long)low);
    
    return 0;
}
