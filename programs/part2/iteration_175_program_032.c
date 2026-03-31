/* Test program to trigger target hook for built-in helper generation */
/* Compile with: gcc -O3 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin test_targhooks.c helper.c -o test_targhooks */

#include <stdio.h>
#include <stdint.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This division may require __divti3 helper on targets without native 128-bit division */
    return a / b;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
__attribute__((nothrow)) 
static void atomic_128bit_op(__int128 *val) {
    __int128 desired, expected;
    
    /* Atomic operation on 128-bit variable - may require helper */
    expected = *val;
    desired = expected + 1;
    
    /* This may generate atomic helper calls */
    __atomic_compare_exchange(val, &expected, &desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Volatile 128-bit variable */
volatile __int128 volatile_128 = 0;

int main() {
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 100;
    __int128 result = 0;
    
    /* 1. Perform 128-bit division - may trigger __divti3 helper generation */
    result = do_128bit_division(a, b);
    
    /* 2. Use volatile 128-bit variable */
    volatile_128 = result;
    
    /* 3. Atomic operation on 128-bit variable */
    atomic_128bit_op((__int128*)&result);
    
    /* 4. OpenMP target region with 128-bit variable */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Simple operation in target region */
        result = result + 1;
    }
    #endif
    
    /* 5. Additional complex 128-bit operations */
    __int128 c = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    __int128 d = 7;
    
    /* Mix of operations that may require different helpers */
    __int128 mod_result = c % d;      /* May need __umodti3 */
    __int128 mul_result = result * 3; /* May need __multi3 */
    
    /* Combine results to prevent optimization */
    result = result + mod_result + mul_result;
    
    /* Print hash to prevent dead code elimination */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    
    printf("Result hash: 0x%016llx%016llx\n", 
           (unsigned long long)high, 
           (unsigned long long)low);
    
    return 0;
}
