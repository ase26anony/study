/* Test program to trigger target hook flags in targhooks.cc */
/* Compile with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin -c test_targhooks.c */
/* Then compile with: gcc -O2 -ftest-coverage -fprofile-arcs -fopenmp -fno-builtin helper_usage.c test_targhooks.o */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline __attribute__((always_inline)) 
__attribute__((nothrow)) 
unsigned __int128 use_128bit_ops(unsigned __int128 a, unsigned __int128 b) 
{
    /* Complex 128-bit operations that likely require helper calls */
    unsigned __int128 result = a / b;      /* May call __udivti3 */
    result += a % b;                       /* May call __umodti3 */
    result *= b;                           /* May call __multi3 */
    return result;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
__attribute__((nothrow))
void process_128bit_values(void)
{
    volatile unsigned __int128 v1 = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 v2 = ((unsigned __int128)0x1111111111111111ULL << 64) | 0x2222222222222222ULL;
    
    /* This division on volatile may trigger helper generation with TREE_THIS_VOLATILE */
    unsigned __int128 div_result = v1 / v2;
    
    /* Atomic operation on 128-bit value */
    unsigned __int128 atomic_val = 0;
    __atomic_store_n(&atomic_val, div_result, __ATOMIC_SEQ_CST);
    
    /* Another atomic operation */
    unsigned __int128 expected = 0;
    unsigned __int128 desired = div_result;
    __atomic_compare_exchange_n(&atomic_val, &expected, desired, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* Use OpenMP to potentially trigger target-specific helpers */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: atomic_val) if(0)  /* if(0) to avoid actual offloading but keep hook calls */
    {
        /* Simple operation inside target region */
        atomic_val = atomic_val + 1;
    }
    #endif
    
    /* Use the inline function */
    unsigned __int128 final_result = use_128bit_ops(atomic_val, v2);
    
    /* Print hash to prevent optimization */
    uint64_t high = (uint64_t)(final_result >> 64);
    uint64_t low = (uint64_t)final_result;
    printf("Result hash: 0x%016lx%016lx\n", high, low);
}

int main(void) 
{
    /* Loop to ensure operations aren't optimized away */
    for (int i = 0; i < 10; i++) {
        process_128bit_values();
    }
    
    /* Additional 128-bit operation in main */
    __int128 signed_a = -((__int128)1 << 126);
    __int128 signed_b = 3;
    __int128 signed_div = signed_a / signed_b;  /* May call __divti3 */
    
    uint64_t high = (uint64_t)(signed_div >> 64);
    uint64_t low = (uint64_t)signed_div;
    printf("Signed division: 0x%016lx%016lx\n", high, low);
    
    return 0;
}
