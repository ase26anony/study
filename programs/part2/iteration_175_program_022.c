/* Test for targhooks.cc uncovered lines - 128-bit helpers with various attributes */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force generation of 128-bit helpers */
#ifdef __cplusplus
extern "C" {
#endif

/* Inline function that uses 128-bit operations - will be included in header */
static inline unsigned __int128 helper_divmod(unsigned __int128 a, unsigned __int128 b) {
    /* Complex 128-bit operations that may need runtime helpers */
    unsigned __int128 quot = a / b;      /* May call __udivti3 */
    unsigned __int128 rem = a % b;       /* May call __umodti3 */
    return quot ^ rem;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
unsigned __int128 __attribute__((nothrow)) atomic_op(unsigned __int128 *ptr) {
    unsigned __int128 desired = 100;
    unsigned __int128 expected = *ptr;
    
    /* Atomic compare-exchange on 128-bit - may need helper */
    __atomic_compare_exchange_n(ptr, &expected, desired, 0,
                               __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return *ptr;
}

#ifdef __cplusplus
}
#endif

/* OpenMP target region if supported */
#ifdef _OPENMP
void omp_target_test(unsigned __int128 *result) {
    #pragma omp target map(tofrom: result[0:1])
    {
        /* 128-bit operation inside target region */
        unsigned __int128 a = 1000;
        unsigned __int128 b = 3;
        *result = a / b;  /* May generate helper declaration */
    }
}
#endif

/* OpenACC parallel region if supported */
#ifdef _OPENACC
void acc_parallel_test(unsigned __int128 *sum) {
    unsigned __int128 local_sum = 0;
    
    #pragma acc parallel copyout(local_sum)
    {
        #pragma acc loop reduction(+:local_sum)
        for(int i = 0; i < 100; i++) {
            local_sum += (unsigned __int128)i * i;
        }
    }
    *sum = local_sum;
}
#endif

int main() {
    volatile unsigned __int128 volatile_var = 100;
    unsigned __int128 regular_var = 1000;
    unsigned __int128 result = 0;
    
    /* 1. Division with volatile operand - may influence TREE_THIS_VOLATILE */
    regular_var = volatile_var / 3;
    
    /* 2. Multiple 128-bit operations in loop */
    for (int i = 1; i < 10; i++) {
        unsigned __int128 a = (unsigned __int128)regular_var * i;
        unsigned __int128 b = (unsigned __int128)i + 1;
        
        /* Call inline helper */
        result ^= helper_divmod(a, b);
        
        /* Direct 128-bit multiplication */
        result *= (unsigned __int128)123456789;
    }
    
    /* 3. Atomic operations on 128-bit */
    unsigned __int128 atomic_var = 50;
    result += atomic_op(&atomic_var);
    
    /* 4. Atomic load/store */
    unsigned __int128 load_result;
    __atomic_load(&atomic_var, &load_result, __ATOMIC_SEQ_CST);
    result ^= load_result;
    
    /* 5. Try OpenMP target if available */
    #ifdef _OPENMP
    unsigned __int128 omp_result = 0;
    omp_target_test(&omp_result);
    result += omp_result;
    #endif
    
    /* 6. Try OpenACC if available */
    #ifdef _OPENACC
    unsigned __int128 acc_result = 0;
    acc_parallel_test(&acc_result);
    result += acc_result;
    #endif
    
    /* Prevent dead code elimination */
    unsigned long long hi = (unsigned long long)(result >> 64);
    unsigned long long lo = (unsigned long long)result;
    
    printf("Result hash: %016llx%016llx\n", hi, lo);
    
    /* Use result to affect return value */
    return (int)((hi ^ lo) & 0x7FFFFFFF);
}
