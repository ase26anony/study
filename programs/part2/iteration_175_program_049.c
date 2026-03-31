/* Test program to trigger target hook for generating helper functions
   with specific tree flags (TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, etc.) */

#include <stdio.h>
#include <stdint.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This division may require __divti3 helper on targets without native 128-bit division */
    return a / b;
}

/* Function marked nothrow to influence TREE_NOTHROW flag on helpers */
__attribute__((nothrow)) static void atomic_128bit_op(__int128 *ptr) {
    __int128 desired = 100;
    __int128 expected = *ptr;
    
    /* Atomic compare-exchange on 128-bit may require helper */
    __atomic_compare_exchange_n(ptr, &expected, desired, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Main driver function */
int main() {
    volatile __int128 volatile_var = 100;  /* TREE_THIS_VOLATILE influence */
    __int128 dividend = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 divisor = 100;
    __int128 result = 0;
    
    /* Loop to ensure helper is used multiple times */
    for (int i = 0; i < 10; i++) {
        /* 128-bit division - may trigger __divti3 helper generation */
        result += do_128bit_division(dividend + i, divisor);
        
        /* 128-bit modulo - may trigger __modti3 helper */
        result += (dividend + i) % divisor;
    }
    
    /* Atomic operations on 128-bit variable */
    __int128 atomic_var = 0;
    __atomic_store_n(&atomic_var, result, __ATOMIC_RELAXED);
    
    /* Call nothrow function with atomic operation */
    atomic_128bit_op(&atomic_var);
    
    /* OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: atomic_var) if(0)  /* if(0) to run on host but still process pragma */
    {
        /* Simple operation in target region */
        atomic_var += 1;
    }
    #endif
    
    /* OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(atomic_var) if(0)  /* if(0) to run on host */
    {
        atomic_var += 2;
    }
    #endif
    
    /* Prevent dead code elimination */
    uint64_t low = (uint64_t)(atomic_var & 0xFFFFFFFFFFFFFFFFULL);
    uint64_t high = (uint64_t)((atomic_var >> 64) & 0xFFFFFFFFFFFFFFFFULL);
    
    printf("Result hash: 0x%016llx%016llx\n", (unsigned long long)high, (unsigned long long)low);
    
    return 0;
}
