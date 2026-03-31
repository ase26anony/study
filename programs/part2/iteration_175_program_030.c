/* Test program to trigger target hook for built-in helper generation */
#include <stdio.h>
#include <stdint.h>

/* Inline function to force helper generation in multiple TUs */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This division may require __divti3 helper */
    return a / b;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
__attribute__((nothrow)) 
static void atomic_128bit_op(__int128 *val) {
    __int128 desired = 100;
    __int128 expected = *val;
    
    /* Atomic compare-exchange on 128-bit may require helper */
    __atomic_compare_exchange_n(val, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Main driver */
int main() {
    volatile __int128 volatile_var = 1000;
    __int128 regular_var = 500;
    __int128 result;
    
    /* 1. 128-bit division - may trigger __divti3 helper */
    result = do_128bit_division(volatile_var, regular_var);
    
    /* 2. Atomic operation on 128-bit - may trigger atomic helper */
    __int128 atomic_var = 0;
    __atomic_store_n(&atomic_var, result, __ATOMIC_SEQ_CST);
    
    /* 3. Another operation to ensure helper is used */
    __int128 product = result * regular_var;
    
    /* 4. Use OpenMP to potentially trigger target helpers */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: product) if(0)
    {
        /* Simple operation in target region */
        product = product + 1;
    }
    #endif
    
    /* 5. Use OpenACC if available */
    #ifdef _OPENACC
    #pragma acc parallel copy(product) if(0)
    {
        product = product * 2;
    }
    #endif
    
    /* 6. Call nothrow function with atomic operation */
    atomic_128bit_op(&product);
    
    /* Prevent dead code elimination */
    uint64_t low = (uint64_t)product;
    uint64_t high = (uint64_t)(product >> 64);
    printf("Result hash: %016lx%016lx\n", high, low);
    
    return 0;
}
