/* Test program to trigger target hook flags in targhooks.cc */
/* Compile with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin -c test_main.c */
/* Then link with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin test_main.o test_aux.o -lgomp */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Include the header with inline functions */
#include "test_helpers.h"

/* Function with nothrow attribute to influence TREE_NOTHROW flag */
int __attribute__((nothrow)) atomic_update(__int128 *ptr, __int128 val) {
    __int128 expected, desired;
    int success;
    
    do {
        expected = *ptr;
        desired = expected + val;
        success = __atomic_compare_exchange(ptr, &expected, &desired, 
                                           0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    } while (!success);
    
    return 0;
}

int main(void) {
    volatile __int128 volatile_var = 100;
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 1000000000000000000ULL;
    __int128 result = 0;
    __int128 atomic_var = 0;
    
    printf("Starting 128-bit operations test...\n");
    
    /* 1. 128-bit division - triggers __divti3 helper */
    result = a / b;
    printf("Division result high: 0x%016llx\n", (unsigned long long)(result >> 64));
    
    /* 2. 128-bit modulo - triggers __modti3 helper */
    result = a % b;
    printf("Modulo result low: 0x%016llx\n", (unsigned long long)result);
    
    /* 3. Atomic operations on 128-bit */
    __atomic_store(&atomic_var, &a, __ATOMIC_SEQ_CST);
    atomic_update(&atomic_var, b);
    
    /* 4. Call inline function from header (forces helper in multiple TUs) */
    result = multiply_128bit(a, b);
    
    /* 5. OpenMP target region with 128-bit variable */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Complex operation that might need helpers */
        result = result / 2;
    }
    #endif
    
    /* 6. Volatile access in loop */
    for (int i = 0; i < 10; i++) {
        volatile_var = volatile_var * 2;
    }
    
    /* 7. More atomic operations */
    __int128 load_result;
    __atomic_load(&atomic_var, &load_result, __ATOMIC_SEQ_CST);
    
    /* Prevent dead code elimination */
    unsigned long long hash = ((unsigned long long)(result >> 64)) ^ 
                             ((unsigned long long)result) ^
                             ((unsigned long long)(load_result >> 64)) ^
                             ((unsigned long long)load_result);
    
    printf("Final hash: 0x%016llx\n", hash);
    
    return 0;
}
