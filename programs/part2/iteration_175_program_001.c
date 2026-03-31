/* Main driver that uses various constructs to trigger built-in helper generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline unsigned __int128 calculate_hash(unsigned __int128 a, unsigned __int128 b) 
    __attribute__((nothrow));

static inline unsigned __int128 calculate_hash(unsigned __int128 a, unsigned __int128 b) {
    /* Complex 128-bit arithmetic that may require helper functions */
    unsigned __int128 result = (a * b) / (a + 1);
    result = result % (b + 1);
    return result;
}

/* Function with atomic operations on 128-bit types */
void atomic_128bit_operation(volatile __int128 *dest, __int128 value) 
    __attribute__((nothrow));

void atomic_128bit_operation(volatile __int128 *dest, __int128 value) {
    __int128 expected = *dest;
    __int128 desired = value;
    
    /* Atomic compare-exchange on 128-bit - may require helper */
    __atomic_compare_exchange(dest, &expected, &desired, 0, 
                             __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Atomic load of volatile 128-bit */
    __int128 loaded = __atomic_load_n(dest, __ATOMIC_ACQUIRE);
    (void)loaded; /* Prevent unused warning */
}

/* OpenMP target region using 128-bit variables */
#pragma omp declare target
__int128 omp_global_var = 0;
#pragma omp end declare target

int main() {
    volatile __int128 volatile_var = 100;
    __int128 regular_var = 1000;
    unsigned __int128 unsigned_var = 5000;
    
    /* 1. Perform 128-bit division - may trigger __divti3/__umodti3 helpers */
    for (int i = 0; i < 10; i++) {
        regular_var = (regular_var * 3) / (volatile_var + 1);
        volatile_var++;
    }
    
    /* 2. Use atomic operations on 128-bit */
    atomic_128bit_operation(&volatile_var, regular_var);
    
    /* 3. Calculate using inline function */
    unsigned __int128 hash = calculate_hash(unsigned_var, (unsigned __int128)regular_var);
    
    /* 4. OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: regular_var)
    {
        regular_var = regular_var * 2;
        omp_global_var = regular_var;
    }
    #endif
    
    /* 5. OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(regular_var)
    {
        regular_var = regular_var + 1;
    }
    #endif
    
    /* Print hash to prevent dead code elimination */
    unsigned long long hash_high = (unsigned long long)(hash >> 64);
    unsigned long long hash_low = (unsigned long long)hash;
    
    printf("Result hash: 0x%016llx%016llx\n", hash_high, hash_low);
    
    /* Additional volatile access in loop */
    for (int i = 0; i < 5; i++) {
        volatile_var = volatile_var / 2;
    }
    
    return 0;
}
