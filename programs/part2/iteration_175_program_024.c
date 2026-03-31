#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Header file to be included in multiple translation units */
#include "triggers.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Function with nothrow attribute to influence TREE_NOTHROW flag */
void nothrow_helper(__int128 *result) __attribute__((nothrow));

#ifdef __cplusplus
}
#endif

void nothrow_helper(__int128 *result) {
    /* Use volatile to influence TREE_THIS_VOLATILE */
    volatile __int128 a = 100;
    __int128 b = 3;
    *result = a / b;  /* This may trigger helper generation */
}

int main() {
    /* Declare volatile __int128 variables */
    volatile __int128 volatile_var = 1000;
    __int128 regular_var = 1234567890123456789LL;
    
    /* Initialize with large values */
    regular_var = (regular_var << 64) | 9876543210987654321LL;
    
    /* 1. Perform 128-bit division (triggers __divti3 or similar) */
    __int128 divisor = 7;
    __int128 division_result = regular_var / divisor;
    
    /* 2. Perform 128-bit modulo (triggers __modti3 or similar) */
    __int128 modulo_result = regular_var % divisor;
    
    /* 3. Use atomic operations on __int128 */
    __int128 atomic_var = 0;
    __int128 expected = 0;
    __int128 desired = division_result;
    
    /* Atomic compare and exchange */
    int success = __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Atomic load */
    __int128 loaded = __atomic_load_n(&atomic_var, __ATOMIC_SEQ_CST);
    
    /* 4. Call nothrow function with volatile operations */
    __int128 nothrow_result;
    nothrow_helper(&nothrow_result);
    
    /* 5. Use inline function from header (multiple translation units) */
    __int128 header_result = perform_128bit_operation(division_result, modulo_result);
    
    /* 6. OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: header_result)
    {
        /* Complex operation inside target region */
        header_result = header_result * 2 - 1;
    }
    #endif
    
    /* 7. OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(header_result)
    {
        #pragma acc loop reduction(+:header_result)
        for (int i = 0; i < 10; i++) {
            header_result += i;
        }
    }
    #endif
    
    /* Combine all results to prevent dead code elimination */
    __int128 final_result = division_result + modulo_result + loaded + 
                           nothrow_result + header_result + volatile_var;
    
    /* Print hash to prevent optimization */
    uint64_t high = (uint64_t)(final_result >> 64);
    uint64_t low = (uint64_t)final_result;
    
    printf("Result hash: 0x%016llx%016llx\n", (unsigned long long)high, 
           (unsigned long long)low);
    
    return 0;
}
