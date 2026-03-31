/* Test program to trigger target hook for helper function generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline __int128 complex_128bit_operation(__int128 a, __int128 b) {
    /* Complex 128-bit division that may require helper functions */
    __int128 result = a / b;
    
    /* Also use modulo which may need different helper */
    __int128 remainder = a % b;
    
    return result + remainder;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
__attribute__((nothrow))
static void atomic_128bit_update(__int128 *ptr, __int128 val) {
    /* Atomic operation on 128-bit variable */
    __int128 expected = *ptr;
    __int128 desired = val;
    
    /* This may trigger atomic helper generation */
    __atomic_compare_exchange(ptr, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* OpenMP target region if supported */
#ifdef _OPENMP
#pragma omp declare target
__int128 omp_target_var = 0;
#pragma omp end declare target
#endif

int main() {
    volatile __int128 volatile_var = 100;
    __int128 regular_var = 50;
    __int128 atomic_var = 0;
    
    /* Loop to ensure operations are not optimized away */
    for (int i = 0; i < 10; i++) {
        /* 128-bit division - may trigger __divti3 helper */
        __int128 div_result = volatile_var / regular_var;
        
        /* Use the inline function */
        __int128 complex_result = complex_128bit_operation(volatile_var, regular_var);
        
        /* Update atomic variable */
        atomic_128bit_update(&atomic_var, complex_result);
        
        volatile_var += div_result;
        regular_var += 1;
    }
    
    /* OpenMP target region */
#ifdef _OPENMP
    #pragma omp target map(tofrom: atomic_var)
    {
        atomic_var = atomic_var * 2;
    }
#endif
    
    /* OpenACC parallel region if supported */
#ifdef _OPENACC
    #pragma acc parallel copy(atomic_var)
    {
        atomic_var = atomic_var + 1;
    }
#endif
    
    /* Prevent dead code elimination by printing hash of result */
    unsigned long long high = (unsigned long long)(atomic_var >> 64);
    unsigned long long low = (unsigned long long)atomic_var;
    
    printf("Result hash: %llx%llx\n", high, low);
    
    return 0;
}
