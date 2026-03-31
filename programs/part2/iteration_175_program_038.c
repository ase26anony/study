/* Test for targhooks.cc uncovered lines - 128-bit helpers with various contexts */
#include <stdio.h>
#include <stdint.h>

/* Force generation of 128-bit helpers by disabling builtins */
#pragma GCC optimize("no-builtin")

/* Inline function that uses 128-bit division - will be included in header */
static inline unsigned __int128 div128_helper(unsigned __int128 a, unsigned __int128 b) {
    return a / b;  /* This should trigger __udivti3 helper generation */
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
unsigned __int128 __attribute__((nothrow)) atomic_op(unsigned __int128 *ptr) {
    unsigned __int128 desired = 100;
    unsigned __int128 expected = *ptr;
    
    /* Atomic compare-exchange on 128-bit - may trigger atomic helper */
    __atomic_compare_exchange_n(ptr, &expected, desired, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return *ptr;
}

/* Volatile 128-bit operation */
volatile unsigned __int128 volatile_op(volatile unsigned __int128 *v) {
    return *v / 2;  /* Division on volatile 128-bit */
}

int main() {
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 b = 1000;
    unsigned __int128 result = 0;
    
    /* 1. Regular 128-bit division - triggers helper generation */
    result = a / b;
    
    /* 2. Modulo operation - different helper */
    result += a % b;
    
    /* 3. Multiplication - yet another helper */
    result *= b;
    
    /* 4. Atomic operations on 128-bit */
    unsigned __int128 atomic_var = 42;
    __atomic_store_n(&atomic_var, result, __ATOMIC_RELAXED);
    result = __atomic_load_n(&atomic_var, __ATOMIC_ACQUIRE);
    
    /* 5. Call nothrow function with atomic operation */
    result = atomic_op(&atomic_var);
    
    /* 6. Volatile 128-bit operation */
    volatile unsigned __int128 volatile_var = 1000000;
    result += volatile_op(&volatile_var);
    
    /* 7. Use inline helper */
    result += div128_helper(a, b);
    
    /* OpenMP target region if supported */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* 128-bit operation inside target region */
        unsigned __int128 tmp = result;
        result = tmp / 100;
    }
    #endif
    
    /* Prevent dead code elimination */
    unsigned long long hi = (unsigned long long)(result >> 64);
    unsigned long long lo = (unsigned long long)result;
    printf("Result hash: %016llx%016llx\n", hi, lo);
    
    return 0;
}
