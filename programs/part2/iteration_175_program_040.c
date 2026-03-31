/* Test program to trigger target hook for helper function generation */
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
    return (a * b) / (a + 1);
}

/* Function with nothrow attribute */
void process_value(unsigned __int128 val) __attribute__((nothrow));

#ifdef __cplusplus
}
#endif

/* OpenMP target region if supported */
#ifdef _OPENMP
#include <omp.h>
#endif

/* Function using atomic operations on 128-bit values */
static void atomic_128bit_ops(unsigned __int128 *shared) {
    unsigned __int128 desired = 100;
    unsigned __int128 expected = *shared;
    
    /* Atomic compare exchange on 128-bit */
    __atomic_compare_exchange_n(shared, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Atomic load */
    unsigned __int128 loaded = __atomic_load_n(shared, __ATOMIC_ACQUIRE);
    
    /* Atomic store */
    __atomic_store_n(shared, loaded + 1, __ATOMIC_RELEASE);
}

/* Nothrow function using volatile 128-bit */
void process_value(unsigned __int128 val) __attribute__((nothrow)) {
    volatile unsigned __int128 volatile_val = val;
    unsigned __int128 temp = volatile_val;
    
    /* Division operation on volatile */
    temp = temp / 1000;
    
    /* Use the value to prevent optimization */
    asm volatile("" : "+r"(temp));
}

int main() {
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 b = ((unsigned __int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    unsigned __int128 result = 0;
    
    /* Loop with 128-bit division - should trigger helper generation */
    for (int i = 0; i < 100; i++) {
        /* Use inline function */
        result += calculate_hash(a + i, b - i);
        
        /* Direct 128-bit modulo operation */
        result = result % (b + 1);
    }
    
    /* Atomic operations on 128-bit */
    atomic_128bit_ops(&result);
    
    /* Process with nothrow function */
    process_value(result);
    
    /* OpenMP target region if available */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result) if(0)  /* if(0) to run on host but still process */
    {
        /* Simple operation in target region */
        result = result / 2;
    }
    #endif
    
    /* Print hash to prevent dead code elimination */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    printf("Result hash: 0x%016llx%016llx\n", 
           (unsigned long long)high, 
           (unsigned long long)low);
    
    return 0;
}
