/* Test program to trigger target hook for built-in helper generation */
#include <stdio.h>
#include <stdint.h>

/* Inline function that uses 128-bit operations - will be included in header */
static inline unsigned __int128 do_128bit_division(unsigned __int128 a, 
                                                   unsigned __int128 b) {
    /* This division may require runtime helper on targets without native 128-bit support */
    return a / b;
}

/* Function marked nothrow to influence TREE_NOTHROW flag */
unsigned __int128 __attribute__((nothrow)) 
atomic_update(unsigned __int128 *ptr, unsigned __int128 val) {
    unsigned __int128 expected, desired;
    
    /* Atomic operations on 128-bit may require helper functions */
    desired = *ptr + val;
    
    /* __atomic_compare_exchange may trigger helper generation */
    __atomic_compare_exchange_n(ptr, &expected, desired, 0, 
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    return desired;
}

/* Volatile variable to influence TREE_THIS_VOLATILE */
volatile unsigned __int128 global_volatile_128 = 0;

int main() {
    unsigned __int128 dividend = ((unsigned __int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    unsigned __int128 divisor = 1000000007;
    unsigned __int128 result = 0;
    unsigned __int128 atomic_var = 1;
    
    /* 1. 128-bit division operation - may trigger __umodti3/__udivti3 helpers */
    result = do_128bit_division(dividend, divisor);
    
    /* 2. Atomic operation on 128-bit variable */
    atomic_update(&atomic_var, result);
    
    /* 3. Operation on volatile 128-bit variable */
    global_volatile_128 = dividend;
    result += global_volatile_128 / divisor;
    
    /* 4. OpenMP target region with 128-bit variable */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        /* Inside target region, compiler may generate device helpers */
        unsigned __int128 local_128 = result;
        for (int i = 0; i < 10; i++) {
            local_128 = local_128 / divisor;
        }
        result = local_128;
    }
    #endif
    
    /* 5. OpenACC parallel region (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel copy(result)
    {
        unsigned __int128 acc_128 = result;
        acc_128 = acc_128 % 1000000009;
        result = acc_128;
    }
    #endif
    
    /* Prevent dead code elimination by printing hash of result */
    unsigned long long hi = (unsigned long long)(result >> 64);
    unsigned long long lo = (unsigned long long)result;
    printf("Result hash: %llx%llx\n", hi, lo);
    
    return 0;
}
