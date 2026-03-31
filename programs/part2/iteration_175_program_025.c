/* Test for targhooks.cc uncovered lines - flags on built-in helpers */
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
    /* This should trigger __umodti3 or similar helper */
    return a % b;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
unsigned __int128 atomic_op(unsigned __int128 *ptr) 
    __attribute__((nothrow));

#ifdef __cplusplus
}
#endif

/* Use volatile to potentially affect TREE_THIS_VOLATILE */
volatile unsigned __int128 global_128 = 0;

/* Atomic operation on 128-bit - may need libcall */
unsigned __int128 atomic_op(unsigned __int128 *ptr) {
    unsigned __int128 expected = *ptr;
    unsigned __int128 desired = expected + 1;
    
    /* This may trigger atomic helper generation */
    __atomic_compare_exchange_n(ptr, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return desired;
}

int main() {
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEFULL << 64) | 
                          0xFEDCBA9876543210ULL;
    unsigned __int128 b = 0x1000000000000000ULL;
    unsigned __int128 result = 0;
    
    /* 1. 128-bit modulo operation - should trigger helper */
    result = do_128bit_division(a, b);
    
    /* 2. Atomic operation on 128-bit */
    unsigned __int128 atomic_var = 100;
    atomic_op(&atomic_var);
    
    /* 3. Use volatile 128-bit variable */
    global_128 = a;
    unsigned __int128 volatile_result = global_128 / b;
    
    /* 4. OpenMP target region with 128-bit operation */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* 128-bit operation inside target region */
        result = result + 1;
    }
    #endif
    
    /* 5. Another 128-bit operation in loop */
    for (int i = 0; i < 10; i++) {
        result = result * b + i;
    }
    
    /* Combine results to prevent optimization */
    unsigned __int128 final = result + atomic_var + volatile_result;
    
    /* Print hash to prevent dead code elimination */
    uint64_t low = (uint64_t)final;
    uint64_t high = (uint64_t)(final >> 64);
    printf("Result hash: %016llx%016llx\n", 
           (unsigned long long)high, 
           (unsigned long long)low);
    
    return 0;
}
