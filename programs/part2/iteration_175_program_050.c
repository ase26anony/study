/* Main test file to trigger target hook for generating helper declarations */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Inline function that uses 128-bit operations - will be included in header */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This division may require runtime helper on targets without native 128-bit support */
    return a / b;
}

/* Function with nothrow attribute */
int __attribute__((nothrow)) atomic_update(__int128 *ptr, __int128 val) {
    __int128 expected, desired;
    expected = *ptr;
    desired = expected + val;
    
    /* Use atomic compare-exchange on 128-bit variable */
    return __atomic_compare_exchange_n(ptr, &expected, desired, 
                                       0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Volatile 128-bit operation */
volatile __int128 volatile_128bit_op(volatile __int128 a, volatile __int128 b) {
    /* Operation on volatile 128-bit may trigger special handling */
    return a * b;
}

#ifdef _OPENMP
/* OpenMP target region using 128-bit variable */
void omp_target_128bit(__int128 *result) {
    __int128 local = 100;
    #pragma omp target map(tofrom: local)
    {
        /* 128-bit operation inside target region */
        local = local / 3;
    }
    *result = local;
}
#endif

int main() {
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 100;
    __int128 result = 0;
    
    /* 1. Perform 128-bit division (may require __divti3 helper) */
    result = do_128bit_division(a, b);
    
    /* 2. Atomic operation on 128-bit variable (may require atomic helper) */
    __int128 atomic_var = 0;
    __atomic_store_n(&atomic_var, result, __ATOMIC_RELAXED);
    
    /* 3. Volatile operation */
    volatile __int128 v1 = 500, v2 = 2;
    __int128 volatile_result = volatile_128bit_op(v1, v2);
    
    /* 4. Call nothrow function with atomic operation */
    atomic_update(&atomic_var, volatile_result);
    
#ifdef _OPENMP
    /* 5. OpenMP target region if supported */
    __int128 omp_result;
    omp_target_128bit(&omp_result);
    atomic_var += omp_result;
#endif
    
    /* 6. Another 128-bit operation: modulo (may require __modti3 helper) */
    __int128 c = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 mod_result = c % (b + 1);
    atomic_var += mod_result;
    
    /* Prevent dead code elimination by printing hash of result */
    uint64_t high = (uint64_t)(atomic_var >> 64);
    uint64_t low = (uint64_t)atomic_var;
    printf("Result hash: 0x%016lx%016lx\n", high, low);
    
    /* Also use the results to prevent optimization */
    if (result != 0 || volatile_result != 0 || mod_result != 0) {
        return 0;
    }
    
    return 1;
}
