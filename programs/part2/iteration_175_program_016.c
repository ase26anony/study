/* Test to trigger target hook for generating helper functions with specific flags */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline unsigned __int128 helper_128bit_ops(unsigned __int128 a, unsigned __int128 b) {
    /* Complex 128-bit operations that may require runtime helpers */
    unsigned __int128 div_result = a / b;          /* May call __udivti3 */
    unsigned __int128 mod_result = a % b;          /* May call __umodti3 */
    unsigned __int128 mul_result = a * b;          /* May call __multi3 */
    
    /* Mix with volatile to influence TREE_THIS_VOLATILE */
    volatile unsigned __int128 volatile_result = div_result + mod_result;
    
    return mul_result + (unsigned __int128)volatile_result;
}

/* Function marked nothrow to influence TREE_NOTHROW */
unsigned __int128 __attribute__((nothrow)) atomic_128bit_op(unsigned __int128 *ptr) {
    unsigned __int128 expected = *ptr;
    unsigned __int128 desired;
    unsigned __int128 result = 0;
    
    /* Atomic operations on 128-bit may require helpers */
    for (int i = 0; i < 10; i++) {
        desired = expected + 1;
        
        /* __atomic_compare_exchange may generate helper calls */
        if (__atomic_compare_exchange_n(ptr, &expected, desired, 
                                        0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
            result = desired;
        } else {
            result = expected;
        }
        
        /* Another atomic operation */
        __atomic_load(ptr, &expected, __ATOMIC_ACQUIRE);
    }
    
    return result;
}

#ifdef _OPENMP
/* OpenMP target region with 128-bit operations */
#pragma omp declare target
unsigned __int128 omp_target_128bit(unsigned __int128 a, unsigned __int128 b) {
    /* Operations inside target region may generate helpers */
    unsigned __int128 local = a;
    for (int i = 0; i < 5; i++) {
        local = local / (b + i);  /* Division may need helper */
        local = local * 3;         /* Multiplication may need helper */
    }
    return local;
}
#pragma omp end declare target
#endif

int main() {
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 b = ((unsigned __int128)0x1111111111111111ULL << 64) | 0x2222222222222222ULL;
    unsigned __int128 result = 0;
    
    /* 1. Use 128-bit division/modulo operations */
    result = helper_128bit_ops(a, b);
    
    /* 2. Atomic operations on 128-bit variable */
    unsigned __int128 atomic_var = 100;
    result += atomic_128bit_op(&atomic_var);
    
    /* 3. More complex 128-bit arithmetic */
    volatile unsigned __int128 volatile_var = a;
    for (int i = 0; i < 3; i++) {
        volatile_var = volatile_var / (b + i);
        result += volatile_var;
    }
    
#ifdef _OPENMP
    /* 4. OpenMP target region (if supported) */
    #pragma omp target map(tofrom: result)
    {
        result = omp_target_128bit(result, b);
    }
#endif
    
    /* Prevent dead code elimination by printing hash of result */
    uint64_t low = (uint64_t)(result & 0xFFFFFFFFFFFFFFFFULL);
    uint64_t high = (uint64_t)(result >> 64);
    
    /* Simple hash mixing */
    uint64_t hash = low ^ (high << 32) ^ (high >> 32);
    printf("Result hash: 0x%016llx\n", (unsigned long long)hash);
    
    return 0;
}
