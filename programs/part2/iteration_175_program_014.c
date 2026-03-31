/* Test program to trigger target hook for generating helper declarations */
#include <stdio.h>
#include <stdint.h>

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline unsigned __int128 helper_128bit_operation(unsigned __int128 a, unsigned __int128 b)
{
    /* Complex 128-bit operation that likely requires runtime helper */
    return (a * a) / (b + 1);
}

/* Function with nothrow attribute */
int __attribute__((nothrow)) atomic_update(volatile __int128 *ptr, __int128 val)
{
    __int128 expected, desired;
    int success;
    
    do {
        expected = *ptr;
        desired = expected + val;
        /* Atomic compare-exchange on 128-bit - may need helper */
        success = __atomic_compare_exchange_n(ptr, &expected, desired, 
                                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    } while (!success);
    
    return success;
}

/* OpenMP target region using 128-bit types */
#ifdef _OPENMP
void omp_128bit_test(void)
{
    __int128 omp_var = 100;
    __int128 omp_result = 0;
    
    #pragma omp target map(tofrom: omp_result)
    {
        /* 128-bit operation inside OpenMP region */
        omp_result = omp_var * omp_var / 37;
    }
    
    /* Use result to prevent optimization */
    printf("OMP result: %llx%llx\n", 
           (unsigned long long)(omp_result >> 64),
           (unsigned long long)omp_result);
}
#endif

int main(void)
{
    volatile __int128 volatile_var = 1000;
    __int128 regular_var = 5000;
    __int128 result = 0;
    
    /* 1. Use inline helper with 128-bit division */
    result = helper_128bit_operation(regular_var, volatile_var);
    
    /* 2. Atomic operation on 128-bit */
    __int128 atomic_var = 100;
    atomic_update(&atomic_var, result);
    
    /* 3. Direct 128-bit modulo operation */
    __int128 a = ((__int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210;
    __int128 b = 0x1000000000000000;
    __int128 mod_result = a % b;  /* This should trigger __modti3 helper */
    
    /* 4. 128-bit multiplication */
    __int128 mul_result = a * b;  /* This might trigger __multti3 helper */
    
    /* 5. OpenMP test if available */
    #ifdef _OPENMP
    omp_128bit_test();
    #endif
    
    /* Combine results and print hash to prevent dead code elimination */
    __int128 final_result = result + atomic_var + mod_result + mul_result;
    
    /* Print as two 64-bit parts */
    unsigned long long hi = (unsigned long long)(final_result >> 64);
    unsigned long long lo = (unsigned long long)final_result;
    
    printf("Result hash: %016llx%016llx\n", hi, lo);
    
    return 0;
}
