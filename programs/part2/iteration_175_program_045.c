/* Test program to trigger target hook for built-in helper declarations */
#include <stdio.h>
#include <stdint.h>

/* Inline function in header to force multiple translation unit usage */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This should trigger __divti3 helper generation */
    return a / b;
}

/* Function with nothrow attribute */
int __attribute__((nothrow)) atomic_update(__int128 *val) {
    __int128 desired, expected;
    
    /* Atomic operations on 128-bit values */
    desired = *val + 1;
    expected = *val;
    
    /* This may trigger atomic helper generation */
    return __atomic_compare_exchange_n(val, &expected, desired, 
                                       0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

#ifdef _OPENMP
/* OpenMP target region with 128-bit operations */
void omp_target_operation(__int128 *result) {
    #pragma omp target map(tofrom: result[0])
    {
        __int128 a = 100;
        __int128 b = 3;
        *result = a / b;  /* Division in target region */
    }
}
#endif

int main() {
    volatile __int128 volatile_val = 100;
    __int128 a = 1234567890123456789LL;
    __int128 b = 9876543210987654321LL;
    __int128 result = 0;
    
    /* 1. 128-bit division - should trigger __divti3 helper */
    result = do_128bit_division(a, b);
    
    /* 2. Atomic operation on 128-bit value */
    __atomic_store_n(&a, result, __ATOMIC_RELAXED);
    
    /* 3. Use volatile 128-bit in division */
    result = volatile_val / 3;
    
    /* 4. Call nothrow function with atomic operation */
    atomic_update(&result);
    
#ifdef _OPENMP
    /* 5. OpenMP target operation if supported */
    omp_target_operation(&result);
#endif
    
    /* Prevent dead code elimination */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    
    printf("Result hash: 0x%016lx%016lx\n", high, low);
    
    /* Additional complex operation to ensure helper is used */
    __int128 c = (__int128)0xFFFFFFFFFFFFFFFFULL;
    __int128 d = (__int128)0x7FFFFFFFFFFFFFFFULL;
    
    /* Mix of operations that might need different helpers */
    result = (c * d) / (a - b);
    result = result % (c + 1);
    
    return (int)(high ^ low);
}
