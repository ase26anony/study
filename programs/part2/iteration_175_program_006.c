/* Test program to trigger target hook for helper function generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This division may require __divti3 helper */
    return a / b;
}

/* Function with nothrow attribute */
int __attribute__((nothrow)) atomic_update(__int128 *val) {
    __int128 desired = *val + 1;
    __int128 expected = *val;
    
    /* Atomic compare-exchange on 128-bit - may require helper */
    return __atomic_compare_exchange_n(val, &expected, desired, 
                                       0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Volatile operation function */
volatile __int128 __attribute__((nothrow)) do_volatile_op(volatile __int128 *ptr) {
    /* Access volatile 128-bit - influences TREE_THIS_VOLATILE */
    return *ptr;
}

int main() {
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x1000000000000000ULL;
    __int128 result = 0;
    
    /* 1. Perform 128-bit division (triggers __divti3 or similar) */
    result = do_128bit_division(a, b);
    
    /* 2. Atomic operations on 128-bit */
    __int128 atomic_val = 100;
    __atomic_store_n(&atomic_val, result, __ATOMIC_SEQ_CST);
    
    /* 3. Volatile access in nothrow context */
    volatile __int128 volatile_var = atomic_val;
    do_volatile_op(&volatile_var);
    
    /* 4. OpenMP target region with 128-bit variable */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Simple operation inside target region */
        result = result + 1;
    }
    #endif
    
    /* 5. More complex 128-bit operations */
    __int128 c = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 d = 0x7FFFFFFFFFFFFFFFULL;
    
    /* Modulo operation - may trigger __modti3 */
    __int128 mod_result = c % d;
    
    /* Multiplication - may trigger __multi3 */
    __int128 mul_result = result * mod_result;
    
    /* 6. Atomic load */
    __int128 loaded = __atomic_load_n(&mul_result, __ATOMIC_ACQUIRE);
    
    /* Prevent dead code elimination */
    uint64_t high = (uint64_t)(loaded >> 64);
    uint64_t low = (uint64_t)loaded;
    
    /* Create a simple hash to print */
    uint64_t hash = high ^ low;
    printf("Result hash: 0x%016llx\n", (unsigned long long)hash);
    
    /* Call atomic update */
    atomic_update(&loaded);
    
    return (int)(hash & 0x7FFFFFFF);
}
