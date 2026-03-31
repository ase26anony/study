#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Inline function that will be included in multiple translation units */
static inline __int128 complex_128bit_operation(__int128 a, __int128 b) {
    /* This division may require __divti3 helper on targets without native 128-bit support */
    __int128 result = a / b;
    
    /* Add some atomic operations which might require atomic helpers */
    __int128 atomic_var = 0;
    __atomic_store_n(&atomic_var, result, __ATOMIC_SEQ_CST);
    
    return __atomic_load_n(&atomic_var, __ATOMIC_SEQ_CST);
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
__attribute__((nothrow)) 
static void process_128bit(volatile __int128 *dest, __int128 src) {
    /* Volatile access combined with 128-bit operation */
    *dest = src + (*dest);
}

int main() {
    volatile __int128 volatile_acc = 0;
    __int128 accumulator = 0;
    
    /* Initialize with values that ensure non-trivial division */
    __int128 numerator = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 denominator = 100;
    
    /* Loop with 128-bit division - may trigger helper generation */
    for (int i = 0; i < 100; i++) {
        __int128 result = complex_128bit_operation(numerator + i, denominator);
        accumulator += result;
        
        /* Process with volatile */
        process_128bit(&volatile_acc, result);
    }
    
    /* Atomic operations on 128-bit values */
    __int128 atomic_val = 0;
    __int128 expected = 0;
    __int128 desired = accumulator;
    
    /* This may trigger atomic helper generation */
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Use OpenMP if available - may generate offloading helpers */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: atomic_val) if(0)
    {
        /* Simple operation in target region */
        atomic_val = atomic_val + 1;
    }
    #endif
    
    /* Use OpenACC if available - may generate parallel region helpers */
    #ifdef _OPENACC
    #pragma acc parallel copy(atomic_val)
    {
        atomic_val = atomic_val * 2;
    }
    #endif
    
    /* Prevent dead code elimination */
    unsigned long long hi = (unsigned long long)(accumulator >> 64);
    unsigned long long lo = (unsigned long long)accumulator;
    
    printf("Result hash: %llx%llx\n", hi, lo);
    printf("Volatile acc: %llx%llx\n", 
           (unsigned long long)(volatile_acc >> 64),
           (unsigned long long)volatile_acc);
    printf("Atomic val: %llx%llx\n",
           (unsigned long long)(atomic_val >> 64),
           (unsigned long long)atomic_val);
    
    return 0;
}
