/* Test program to trigger target hooks for helper function generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that uses 128-bit operations - will be included in header */
static inline unsigned __int128 calculate_hash(unsigned __int128 a, unsigned __int128 b) {
    /* Complex 128-bit arithmetic that may require helper functions */
    unsigned __int128 result = a / (b + 1);  /* May call __udivti3 */
    result = result % (a | 1);               /* May call __umodti3 */
    return result;
}

/* Function with nothrow attribute */
int __attribute__((nothrow)) atomic_update(unsigned __int128 *dest, unsigned __int128 val) {
    unsigned __int128 expected, desired;
    
    /* Atomic operations on 128-bit values */
    __atomic_load(dest, &expected, __ATOMIC_ACQUIRE);
    
    do {
        desired = expected + val;
    } while (!__atomic_compare_exchange(dest, &expected, &desired, 
                                        0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));
    
    return 0;
}

/* Volatile 128-bit operations */
volatile unsigned __int128 global_volatile_128 = 0;

int main(void) {
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 b = ((unsigned __int128)0x1111111111111111ULL << 64) | 0x2222222222222222ULL;
    unsigned __int128 result = 0;
    
    /* 1. 128-bit division/modulo operations */
    printf("Testing 128-bit arithmetic operations...\n");
    
    for (int i = 0; i < 10; i++) {
        /* Division may trigger __udivti3 helper generation */
        unsigned __int128 quotient = a / (b + i);
        
        /* Modulo may trigger __umodti3 helper generation */
        unsigned __int128 remainder = a % (b + i + 1);
        
        result += quotient + remainder;
    }
    
    /* 2. Atomic operations on 128-bit values */
    printf("Testing atomic operations on 128-bit values...\n");
    atomic_update(&result, a);
    
    /* 3. Volatile 128-bit operations */
    printf("Testing volatile 128-bit operations...\n");
    global_volatile_128 = result;
    unsigned __int128 volatile_read = global_volatile_128;
    result ^= volatile_read;
    
    /* 4. Use inline function with 128-bit operations */
    printf("Testing inline function with 128-bit operations...\n");
    result = calculate_hash(result, a);
    
    /* 5. OpenMP target region (if supported) */
#ifdef _OPENMP
    printf("Testing OpenMP target region...\n");
    #pragma omp target map(tofrom: result)
    {
        /* Simple operation in target region */
        result = result * 3 + 1;
    }
#endif
    
    /* 6. OpenACC parallel region (if supported) */
#ifdef _OPENACC
    printf("Testing OpenACC parallel region...\n");
    #pragma acc parallel copy(result)
    {
        result = result / 2;
    }
#endif
    
    /* Print result to prevent optimization */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    printf("Final result hash: 0x%016llx%016llx\n", 
           (unsigned long long)high, (unsigned long long)low);
    
    return 0;
}
