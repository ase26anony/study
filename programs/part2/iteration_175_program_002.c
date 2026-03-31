/* Main test file to trigger target hook for helper function generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that uses 128-bit operations - will be included in header */
static inline unsigned __int128 calculate_hash(unsigned __int128 a, unsigned __int128 b) {
    /* Complex 128-bit operations that may require helper functions */
    unsigned __int128 result = a / b;  /* May call __udivti3 */
    result += a % b;                   /* May call __umodti3 */
    result *= b;                       /* May call __multi3 */
    return result;
}

/* Function with nothrow attribute */
int __attribute__((nothrow)) atomic_update(unsigned __int128 *ptr, unsigned __int128 val) {
    unsigned __int128 expected, desired;
    
    /* Atomic operations on 128-bit values */
    __atomic_load(ptr, &expected, __ATOMIC_SEQ_CST);
    
    do {
        desired = expected + val;
    } while (!__atomic_compare_exchange(ptr, &expected, &desired, 
                                        0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED));
    
    return 0;
}

/* Volatile 128-bit variable */
volatile unsigned __int128 volatile_128 = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;

int main() {
    unsigned __int128 a = ((unsigned __int128)0xDEADBEEFCAFEBABEULL << 64) | 0x123456789ABCDEF0ULL;
    unsigned __int128 b = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 result = 0;
    
    printf("Testing 128-bit operations to trigger target hooks...\n");
    
    /* 1. Perform 128-bit division (may generate __udivti3) */
    result = a / b;
    
    /* 2. Use volatile 128-bit variable */
    volatile_128 = result;
    unsigned __int128 temp = volatile_128;
    
    /* 3. Atomic operations (may generate atomic helper calls) */
    atomic_update(&result, b);
    
    /* 4. Complex calculation using inline function */
    result = calculate_hash(a, b);
    
    /* 5. Loop with 128-bit operations */
    for (int i = 0; i < 10; i++) {
        result = result / ((unsigned __int128)i + 1);
        result += a % ((unsigned __int128)i + 2);
    }
    
    /* 6. OpenMP target region (if supported) */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Simple operation in target region */
        result = result / 2;
    }
    #endif
    
    /* 7. OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(result)
    {
        result = result + 1;
    }
    #endif
    
    /* Prevent dead code elimination and print result */
    uint64_t high = (uint64_t)(result >> 64);
    uint64_t low = (uint64_t)result;
    
    printf("Result high 64 bits: 0x%016llx\n", (unsigned long long)high);
    printf("Result low 64 bits:  0x%016llx\n", (unsigned long long)low);
    
    /* Create a simple hash to verify computation */
    unsigned long long hash = high ^ low;
    printf("Final hash: 0x%016llx\n", (unsigned long long)hash);
    
    return (hash != 0) ? 0 : 1;
}
