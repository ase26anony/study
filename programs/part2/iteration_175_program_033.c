/* Test to trigger target hook for built-in helper generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force generation of 128-bit helpers */
#ifdef __cplusplus
extern "C" {
#endif

/* Inline function that uses 128-bit operations - will be included in header */
static inline unsigned __int128 do_128bit_division(unsigned __int128 a, 
                                                   unsigned __int128 b) {
    return a / b;  /* Should trigger __udivti3 helper */
}

/* Function with nothrow attribute */
int __attribute__((nothrow)) atomic_update(unsigned __int128 *ptr, 
                                           unsigned __int128 val);

#ifdef __cplusplus
}
#endif

/* Header-like inline function for multiple translation units */
__attribute__((always_inline)) 
static inline unsigned __int128 complex_128bit_op(unsigned __int128 x, 
                                                  unsigned __int128 y) {
    /* Mix of operations that might need helpers */
    unsigned __int128 div = x / y;          /* __udivti3 */
    unsigned __int128 mod = x % y;          /* __umodti3 */
    return div + mod;
}

/* Volatile 128-bit variable */
volatile unsigned __int128 g_volatile_128 = ((unsigned __int128)1 << 64) + 12345;

int main() {
    unsigned __int128 result = 0;
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 
                          0xFEDCBA9876543210ULL;
    unsigned __int128 b = ((unsigned __int128)0x1000000000000000ULL << 64) | 
                          0x1ULL;
    
    /* 1. 128-bit division operation */
    result = do_128bit_division(a, b);
    
    /* 2. Complex 128-bit operation */
    result = complex_128bit_op(a, b);
    
    /* 3. Atomic operation on 128-bit variable */
    unsigned __int128 atomic_var = 0;
    unsigned __int128 desired = ((unsigned __int128)1 << 64);
    unsigned __int128 expected = 0;
    
    /* Use GCC built-in atomic operations */
    __atomic_store(&atomic_var, &desired, __ATOMIC_SEQ_CST);
    __atomic_load(&atomic_var, &result, __ATOMIC_SEQ_CST);
    
    /* Atomic compare-exchange */
    __atomic_compare_exchange(&atomic_var, &expected, &desired, 
                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* 4. Operation with volatile 128-bit variable */
    unsigned __int128 temp = g_volatile_128;
    result = result / (temp + 1);
    
    /* 5. OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Simple operation in target region */
        result = result / 2;
    }
    #endif
    
    /* 6. OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(result)
    {
        result = result % b;
    }
    #endif
    
    /* Prevent dead code elimination */
    unsigned long long hi = (unsigned long long)(result >> 64);
    unsigned long long lo = (unsigned long long)result;
    
    /* Create a hash to print */
    unsigned long long hash = hi ^ lo;
    printf("Result hash: 0x%016llx\n", hash);
    
    return (int)(hash & 0x7FFFFFFF);
}

/* Nothrow function implementation */
int __attribute__((nothrow)) atomic_update(unsigned __int128 *ptr, 
                                           unsigned __int128 val) {
    unsigned __int128 old;
    __atomic_load(ptr, &old, __ATOMIC_RELAXED);
    __atomic_store(ptr, &val, __ATOMIC_RELAXED);
    return old == val;
}
