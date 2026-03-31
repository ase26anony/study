#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function in header to potentially generate external declarations */
static inline __int128 do_128bit_division(__int128 a, __int128 b) {
    /* This may trigger generation of __divti3 helper */
    return a / b;
}

/* Function with nothrow attribute */
__attribute__((nothrow)) 
static void atomic_128bit_op(__int128 *val) {
    __int128 desired = 100;
    __int128 expected = *val;
    
    /* Atomic compare-exchange on 128-bit may need helper */
    __atomic_compare_exchange(val, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* Volatile operation that may influence TREE_THIS_VOLATILE */
static __int128 volatile_op(volatile __int128 *v) {
    return *v + 1;
}

int main() {
    /* Declare and initialize 128-bit variables */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 100;
    volatile __int128 volatile_var = 42;
    
    /* 128-bit division - may trigger __divti3 helper */
    __int128 result = do_128bit_division(a, b);
    
    /* Atomic operation on 128-bit */
    __int128 atomic_val = 50;
    atomic_128bit_op(&atomic_val);
    
    /* Volatile operation */
    __int128 volatile_result = volatile_op(&volatile_var);
    
    /* OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        result = result * 2;
    }
    #endif
    
    /* Print result to prevent optimization */
    unsigned long long hi = (unsigned long long)(result >> 64);
    unsigned long long lo = (unsigned long long)result;
    
    printf("Result: 0x%016llx%016llx\n", hi, lo);
    printf("Atomic val: 0x%016llx%016llx\n", 
           (unsigned long long)(atomic_val >> 64),
           (unsigned long long)atomic_val);
    printf("Volatile result: 0x%016llx%016llx\n",
           (unsigned long long)(volatile_result >> 64),
           (unsigned long long)volatile_result);
    
    return 0;
}
