/* Main driver program to trigger target hook for helper function generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Include header with inline functions to force external declarations */
#include "triggers.h"

/* Function marked nothrow to influence TREE_NOTHROW flag */
void process_value(__int128 val) __attribute__((nothrow));

/* Global volatile __int128 to influence TREE_THIS_VOLATILE */
volatile __int128 global_volatile_int128 = 0;

/* Atomic __int128 variable for atomic operations */
__int128 atomic_var = 0;

void process_value(__int128 val) {
    /* This function is marked nothrow, operations inside may influence flags */
    __int128 temp = val;
    
    /* Perform 128-bit division - may trigger helper generation */
    temp = temp / ((__int128)100 + 1);
    
    /* Atomic operation on __int128 */
    __int128 expected = atomic_var;
    __int128 desired = temp;
    __atomic_compare_exchange(&atomic_var, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

int main() {
    /* Initialize 128-bit values */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = ((__int128)0x1000000000000000ULL << 64) | 0x0000000000000001ULL;
    __int128 result = 0;
    
    /* Use volatile variable */
    global_volatile_int128 = a;
    
    /* Loop with 128-bit operations to ensure usage */
    for (int i = 0; i < 10; i++) {
        /* Complex 128-bit operation that may require helper */
        result = perform_complex_128bit_op(a, b, i);
        
        /* Update volatile */
        global_volatile_int128 += result;
        
        /* Call nothrow function */
        process_value(result);
    }
    
    /* Use inline function from header */
    result = inline_128bit_mul(result, b);
    
    /* Atomic load of __int128 */
    __int128 loaded = 0;
    __atomic_load(&atomic_var, &loaded, __ATOMIC_SEQ_CST);
    
    /* Print hash of result to prevent optimization */
    unsigned long long high = (unsigned long long)(result >> 64);
    unsigned long long low = (unsigned long long)result;
    printf("Result hash: 0x%016llx%016llx\n", high, low);
    
    /* Try OpenMP target region if supported */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        result = result / ((__int128)2);
    }
    #endif
    
    /* Try OpenACC if supported */
    #ifdef _OPENACC
    #pragma acc parallel copy(result)
    {
        result = result + ((__int128)1);
    }
    #endif
    
    /* Final print to use result */
    high = (unsigned long long)(result >> 64);
    low = (unsigned long long)result;
    printf("Final result: 0x%016llx%016llx\n", high, low);
    
    return 0;
}
