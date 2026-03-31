/* Test program to trigger target hook for built-in helper generation */
/* Compile with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin -c test_targhooks.c */
/* Also compile with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin -c test_helper.c */
/* Link with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin test_targhooks.o test_helper.o -lgomp */

#include <stdio.h>
#include <stdint.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This division may require __divti3 helper */
    return a / b;
}

/* Function with nothrow attribute */
static __int128 atomic_op_nothrow(__int128 *ptr) __attribute__((nothrow));

static __int128 atomic_op_nothrow(__int128 *ptr) {
    __int128 old_val, new_val;
    
    /* Atomic operation on 128-bit may require helper */
    old_val = __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
    new_val = old_val + 1;
    
    /* Try atomic compare-exchange - may generate helpers */
    __atomic_compare_exchange_n(ptr, &old_val, new_val, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return old_val;
}

/* OpenMP target region using 128-bit variables */
#ifdef _OPENMP
static void omp_target_128bit(void) {
    __int128 result = 0;
    volatile __int128 volatile_var = 100;
    
    #pragma omp target map(tofrom: result)
    {
        /* Use volatile variable inside target region */
        __int128 temp = volatile_var;
        
        /* Perform 128-bit operation inside target region */
        result = temp * 2;
        
        /* Modulo operation may require __modti3 helper */
        if (result != 0) {
            result = result % 5;
        }
    }
    
    /* Use result to prevent optimization */
    printf("Target result: %lld\n", (long long)result);
}
#endif

int main(void) {
    volatile __int128 volatile_128 = 1000;
    __int128 dividend = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 divisor = 100;
    __int128 result;
    
    /* 1. 128-bit division - may trigger __divti3 helper generation */
    result = dividend / divisor;
    
    /* 2. Use volatile variable in calculation */
    result += volatile_128;
    
    /* 3. Atomic operation on 128-bit variable */
    __int128 atomic_var = 500;
    __int128 atomic_result = atomic_op_nothrow(&atomic_var);
    
    /* 4. Modulo operation - may trigger __modti3 helper */
    __int128 modulo_result = dividend % 37;
    
    /* 5. OpenMP target region with 128-bit operations */
    #ifdef _OPENMP
    omp_target_128bit();
    #endif
    
    /* 6. Loop with mixed operations to ensure helpers are used */
    for (int i = 0; i < 10; i++) {
        /* Multiplication may trigger __multi3 helper */
        result = result * 2;
        
        /* Use atomic operation */
        atomic_op_nothrow(&atomic_var);
    }
    
    /* 7. Try complex atomic exchange */
    __int128 expected = atomic_var;
    __int128 desired = expected + 100;
    __atomic_compare_exchange_n(&atomic_var, &expected, desired,
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Prevent dead code elimination */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    
    printf("Result hash: 0x%016llx%016llx\n", 
           (unsigned long long)high, (unsigned long long)low);
    printf("Atomic result: 0x%016llx%016llx\n",
           (unsigned long long)(atomic_result >> 64),
           (unsigned long long)atomic_result);
    printf("Modulo result: 0x%016llx%016llx\n",
           (unsigned long long)(modulo_result >> 64),
           (unsigned long long)modulo_result);
    
    return 0;
}
