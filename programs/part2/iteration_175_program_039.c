/* Test to trigger target hook for generating helper declarations with specific flags */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline unsigned __int128 helper_128bit_operation(unsigned __int128 a, unsigned __int128 b)
{
    /* Complex 128-bit operation that likely requires runtime helper */
    unsigned __int128 result = a / b;  /* May call __udivti3 */
    result += a % b;                   /* May call __umodti3 */
    return result;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
unsigned __int128 __attribute__((nothrow)) atomic_128bit_op(
    volatile unsigned __int128 *ptr, 
    unsigned __int128 val)
{
    unsigned __int128 old;
    /* Atomic operation on 128-bit may require helper */
    __atomic_load(ptr, &old, __ATOMIC_SEQ_CST);
    
    unsigned __int128 desired = old + val;
    while (!__atomic_compare_exchange(ptr, &old, &desired, 
                                      0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        desired = old + val;
    }
    return old;
}

#ifdef _OPENMP
/* OpenMP target region with 128-bit variable */
void omp_128bit_test(unsigned __int128 *result)
{
    unsigned __int128 local = 0;
    
    #pragma omp target map(tofrom: local)
    {
        /* 128-bit operation inside target region */
        unsigned __int128 a = 100;
        unsigned __int128 b = 3;
        local = a / b;  /* May generate helper declaration */
    }
    
    *result = local;
}
#endif

int main(void)
{
    volatile unsigned __int128 volatile_var = 100;
    unsigned __int128 regular_var = 50;
    unsigned __int128 result = 0;
    
    /* 1. 128-bit division/modulo - likely triggers helper generation */
    for (int i = 0; i < 10; i++) {
        regular_var = helper_128bit_operation(regular_var + i, volatile_var);
    }
    
    /* 2. Atomic operation on 128-bit */
    result = atomic_128bit_op(&volatile_var, regular_var);
    
    /* 3. Another 128-bit operation */
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 b = 0x1000000000000000ULL;
    unsigned __int128 div_result = a / b;  /* Direct 128-bit division */
    unsigned __int128 mod_result = a % b;  /* Direct 128-bit modulo */
    
    result += div_result + mod_result;
    
    #ifdef _OPENMP
    /* 4. OpenMP target region if available */
    unsigned __int128 omp_result;
    omp_128bit_test(&omp_result);
    result += omp_result;
    #endif
    
    /* Prevent dead code elimination and print result */
    /* Split 128-bit result into two 64-bit parts for printing */
    uint64_t lower = (uint64_t)(result & 0xFFFFFFFFFFFFFFFFULL);
    uint64_t upper = (uint64_t)(result >> 64);
    
    printf("Result hash: 0x%016llx%016llx\n", 
           (unsigned long long)upper, 
           (unsigned long long)lower);
    
    return 0;
}
