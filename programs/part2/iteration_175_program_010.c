/* Main driver program to trigger target hook flags */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Include the header with inline functions */
#include "triggers.h"

/* Function with nothrow attribute to influence TREE_NOTHROW flag */
int __attribute__((nothrow)) safe_divide(__int128 a, __int128 b, __int128 *result) {
    if (b == 0) return -1;
    *result = a / b;  /* This may trigger __divti3 helper */
    return 0;
}

/* Volatile variable to influence TREE_THIS_VOLATILE flag */
volatile __int128 global_volatile_var = 0;

int main() {
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = 0x1000000000000000ULL;
    __int128 result = 0;
    __int128 atomic_var = 0;
    
    printf("Testing 128-bit operations to trigger target hooks...\n");
    
    /* 1. Trigger 128-bit division helper (likely __divti3) */
    result = a / b;
    printf("Division result high: 0x%llx, low: 0x%llx\n", 
           (unsigned long long)(result >> 64),
           (unsigned long long)result);
    
    /* 2. Trigger modulo helper (likely __modti3) */
    result = a % b;
    printf("Modulo result high: 0x%llx, low: 0x%llx\n",
           (unsigned long long)(result >> 64),
           (unsigned long long)result);
    
    /* 3. Use volatile variable in operation */
    global_volatile_var = a;
    result = global_volatile_var / b;
    
    /* 4. Use nothrow function */
    safe_divide(a, b, &result);
    
    /* 5. Atomic operations on 128-bit variable */
    __atomic_store_n(&atomic_var, a, __ATOMIC_SEQ_CST);
    __int128 expected = a;
    __int128 desired = a + 1;
    __atomic_compare_exchange_n(&atomic_var, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    
    /* 6. Call inline function from header (multiple translation units) */
    result = multiply_128bit(a, b);
    
    /* 7. Use OpenMP target region if supported */
    #ifdef _OPENMP
    #pragma omp target map(tofrom: result)
    {
        result = result * 2;
    }
    #endif
    
    /* 8. Complex 128-bit multiplication */
    unsigned __int128 ua = (unsigned __int128)a;
    unsigned __int128 ub = (unsigned __int128)b;
    unsigned __int128 uresult = ua * ub;  /* May trigger __multi3 helper */
    
    /* 9. Mix operations in loop to ensure usage */
    for (int i = 0; i < 10; i++) {
        result = result / (b + i);
        TREE_USED(result);  /* Ensure variable is marked used */
    }
    
    /* Prevent dead code elimination */
    volatile __int128 final_result = result + uresult;
    
    /* Print hash to prevent optimization */
    uint64_t hash[2];
    hash[0] = (uint64_t)(final_result >> 64);
    hash[1] = (uint64_t)final_result;
    printf("Final hash: 0x%016llx%016llx\n", hash[0], hash[1]);
    
    return 0;
}
