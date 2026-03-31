/* Test program to trigger target hook helper generation with specific flags */
/* Compile with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin -c test_targhooks_main.c */
/* Then link with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin test_targhooks_main.o test_targhooks_aux.o -lgomp */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Include header with inline function to force multiple translation unit usage */
#include "test_targhooks_common.h"

/* Function marked nothrow to influence TREE_NOTHROW flag */
static void process_int128(__int128 a, __int128 b) __attribute__((nothrow));

/* Volatile variable to influence TREE_THIS_VOLATILE */
static volatile __int128 global_volatile_int128 = 0;

/* Atomic variable for atomic operations */
static __int128 global_atomic_int128 = 0;

void process_int128(__int128 a, __int128 b) {
    /* This function is marked nothrow, any helpers generated inside should inherit this */
    
    /* 1. 128-bit division - likely requires __divti3 helper */
    __int128 quotient = a / b;
    
    /* 2. 128-bit modulo - likely requires __modti3 helper */
    __int128 remainder = a % b;
    
    /* 3. Use volatile variable in computation */
    global_volatile_int128 = quotient + remainder;
    
    /* 4. Atomic operation on 128-bit variable */
    __int128 expected = global_atomic_int128;
    __int128 desired = quotient;
    
    /* This may generate atomic helper calls */
    __atomic_compare_exchange(&global_atomic_int128, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#ifdef _OPENMP
/* OpenMP target region that uses 128-bit operations */
void omp_target_operation(void) {
    __int128 omp_var = 100;
    
    #pragma omp target map(tofrom: omp_var)
    {
        /* 128-bit operation inside OpenMP target region */
        __int128 local = 50;
        omp_var = omp_var / local;  /* May require helper in device code */
        omp_var = omp_var * local;
    }
    
    /* Use the result to prevent optimization */
    global_volatile_int128 += omp_var;
}
#endif

int main(void) {
    /* Initialize with values that won't cause overflow in division */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x1000000000000000ULL;
    
    printf("Testing 128-bit operations to trigger helper generation...\n");
    
    /* Call inline function from header (forces helper in multiple TUs) */
    __int128 result1 = compute_with_int128(a, b);
    
    /* Call nothrow function with 128-bit operations */
    process_int128(a, b);
    
    /* Additional 128-bit multiplication (may require __multi3) */
    __int128 product = a * b;
    
    /* Use atomic load */
    __int128 atomic_val = __atomic_load_n(&global_atomic_int128, __ATOMIC_SEQ_CST);
    
    #ifdef _OPENMP
    /* Trigger OpenMP target helper generation if available */
    omp_target_operation();
    #endif
    
    /* Combine results and print hash to prevent dead code elimination */
    __int128 final_result = result1 + product + atomic_val + global_volatile_int128;
    
    /* Print as two 64-bit parts */
    uint64_t high = (uint64_t)(final_result >> 64);
    uint64_t low = (uint64_t)final_result;
    
    printf("Result hash: 0x%016llx%016llx\n", 
           (unsigned long long)high, (unsigned long long)low);
    
    /* Call function from another translation unit */
    external_int128_operation();
    
    return 0;
}
