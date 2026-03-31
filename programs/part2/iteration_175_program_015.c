/* Test to trigger target hook for generating helper functions with specific flags */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force generation of 128-bit helpers */
#ifdef __cplusplus
extern "C" {
#endif

/* Inline function that uses 128-bit operations - will be included in header */
static inline unsigned __int128 calculate_hash(unsigned __int128 a, unsigned __int128 b) {
    /* Complex 128-bit operation that likely requires helper */
    return (a * a + b * b) / (a + 1);
}

#ifdef __cplusplus
}
#endif

/* Function marked nothrow to influence TREE_NOTHROW flag */
int __attribute__((nothrow)) safe_operation(volatile __int128 *dest, __int128 src) {
    /* Atomic operation on 128-bit variable */
    __int128 expected = *dest;
    __int128 desired = src;
    
    /* This may trigger atomic helper generation */
    return __atomic_compare_exchange(dest, &expected, &desired, 0, 
                                     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

int main() {
    volatile __int128 volatile_var = 0;
    __int128 var1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 var2 = ((__int128)0x1122334455667788ULL << 64) | 0x99AABBCCDDEEFF00ULL;
    
    /* 1. 128-bit division - may generate __divti3 or similar */
    __int128 div_result = var1 / (var2 + 1);
    
    /* 2. 128-bit modulo - may generate __modti3 or similar */
    __int128 mod_result = var1 % (var2 | 1);
    
    /* 3. Atomic operations on 128-bit */
    __atomic_store(&volatile_var, &div_result, __ATOMIC_RELEASE);
    
    /* 4. Call nothrow function with volatile 128-bit */
    safe_operation(&volatile_var, mod_result);
    
    /* 5. Use OpenMP if available */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: volatile_var) if(0)
    {
        /* Simple operation in target region */
        volatile_var = volatile_var + 1;
    }
    #endif
    
    /* 6. Use inline function from "header" */
    unsigned __int128 hash = calculate_hash(
        (unsigned __int128)var1, 
        (unsigned __int128)var2
    );
    
    /* Prevent dead code elimination */
    unsigned long long hi = (unsigned long long)(hash >> 64);
    unsigned long long lo = (unsigned long long)hash;
    
    printf("Result hash: 0x%016llx%016llx\n", hi, lo);
    printf("Div result exists: %d\n", div_result != 0);
    printf("Mod result exists: %d\n", mod_result != 0);
    
    return 0;
}
