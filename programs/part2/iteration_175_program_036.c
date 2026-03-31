/* Test to trigger target hook for helper function generation with specific flags */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline unsigned __int128 complex_128bit_op(unsigned __int128 a, 
                                                  unsigned __int128 b) 
{
    /* Complex 128-bit operation that likely requires helper functions */
    unsigned __int128 result = a / b;  /* May call __udivti3 */
    result += a % b;                   /* May call __umodti3 */
    return result;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
unsigned __int128 __attribute__((nothrow)) 
atomic_128bit_update(unsigned __int128 *ptr, unsigned __int128 val) 
{
    unsigned __int128 expected, desired;
    
    /* Atomic compare-exchange on 128-bit - may need atomic helpers */
    expected = *ptr;
    desired = expected + val;
    
    while (!__atomic_compare_exchange_n(ptr, &expected, desired, 
                                        0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        desired = expected + val;
    }
    
    return desired;
}

/* Volatile 128-bit operation */
unsigned __int128 volatile_op(volatile unsigned __int128 *v) 
{
    /* Access volatile 128-bit - may influence TREE_THIS_VOLATILE */
    unsigned __int128 temp = *v;
    temp = temp * 3ULL;  /* May call __multi3 */
    *v = temp;
    return temp;
}

int main() 
{
    volatile unsigned __int128 volatile_var = 100;
    unsigned __int128 regular_var = 10000000000000000000ULL;
    unsigned __int128 divisor = 3;
    unsigned __int128 result = 0;
    
    /* 1. Use complex 128-bit operations */
    for (int i = 0; i < 10; i++) {
        result += complex_128bit_op(regular_var + i, divisor);
    }
    
    /* 2. Atomic operations on 128-bit */
    unsigned __int128 atomic_var = 0;
    result += atomic_128bit_update(&atomic_var, result);
    
    /* 3. Volatile 128-bit operation */
    result += volatile_op(&volatile_var);
    
    /* 4. Try OpenMP target region if supported */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Additional 128-bit operation inside target region */
        unsigned __int128 target_var = result;
        target_var = target_var / 7ULL;
        result = target_var;
    }
    #endif
    
    /* 5. Try OpenACC if supported */
    #ifdef _OPENACC
    #pragma acc parallel copy(result)
    {
        unsigned __int128 acc_var = result;
        acc_var = acc_var % 11ULL;
        result = acc_var;
    }
    #endif
    
    /* Prevent dead code elimination by printing hash of result */
    unsigned long long high = (unsigned long long)(result >> 64);
    unsigned long long low = (unsigned long long)result;
    
    printf("Result hash: %llx%llx\n", high, low);
    printf("Volatile var: %llx%llx\n", 
           (unsigned long long)(volatile_var >> 64),
           (unsigned long long)volatile_var);
    
    return 0;
}
