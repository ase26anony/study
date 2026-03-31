/* main.c - Main driver program */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Include the header with inline functions */
#include "triggers.h"

/* Function with nothrow attribute to influence TREE_NOTHROW flag */
int __attribute__((nothrow)) safe_divide(__int128 a, __int128 b, __int128 *result) {
    if (b != 0) {
        *result = a / b;  /* This may trigger __divti3 helper */
        return 1;
    }
    return 0;
}

/* Function using atomic operations on __int128 */
void atomic_update(__int128 *shared) {
    __int128 desired = 100;
    __int128 expected = *shared;
    
    /* This may trigger atomic helper functions */
    __atomic_compare_exchange_n(shared, &expected, desired, 
                                0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

/* OpenMP target region using __int128 */
#ifdef _OPENMP
void omp_target_operation(void) {
    __int128 arr[10];
    
    #pragma omp target map(tofrom: arr[0:10])
    {
        #pragma omp parallel for
        for (int i = 0; i < 10; i++) {
            arr[i] = (__int128)i * i * i;
        }
    }
    
    /* Use the result to prevent optimization */
    volatile __int128 sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
}
#endif

/* OpenACC parallel region */
#ifdef _OPENACC
void acc_parallel_operation(void) {
    __int128 acc_sum = 0;
    
    #pragma acc parallel copy(acc_sum)
    {
        #pragma acc loop reduction(+:acc_sum)
        for (int i = 0; i < 100; i++) {
            acc_sum += i;
        }
    }
}
#endif

int main(void) {
    /* Declare volatile __int128 to influence TREE_THIS_VOLATILE */
    volatile __int128 volatile_num = 100;
    __int128 num1 = 1234567890123456789;
    __int128 num2 = 987654321;
    __int128 result = 0;
    
    /* 1. Perform 128-bit division (may trigger __divti3) */
    result = num1 / num2;
    
    /* 2. Use inline function from header (forces helper in multiple TUs) */
    result += divide_128bit(num1, num2);
    
    /* 3. Call nothrow function with division */
    safe_divide(num1, num2, &result);
    
    /* 4. Use atomic operations */
    __int128 atomic_var = 0;
    __atomic_store_n(&atomic_var, result, __ATOMIC_RELAXED);
    atomic_update(&atomic_var);
    
    /* 5. Perform modulo operation (may trigger __modti3) */
    __int128 mod_result = num1 % num2;
    
    /* 6. Use volatile variable in operation */
    result += volatile_num * mod_result;
    
    /* 7. Call functions that may trigger OpenMP/OpenACC helpers */
    #ifdef _OPENMP
    omp_target_operation();
    #endif
    
    #ifdef _OPENACC
    acc_parallel_operation();
    #endif
    
    /* 8. Use multiplication (may trigger __multi3) */
    __int128 product = num1 * num2;
    result += product;
    
    /* 9. Complex expression mixing operations */
    __int128 complex_expr = (num1 * num2) / (num1 - num2) % (num2 + 1);
    result += complex_expr;
    
    /* Prevent dead code elimination by printing hash of result */
    /* Split __int128 into two 64-bit parts for printing */
    uint64_t low = (uint64_t)(result & 0xFFFFFFFFFFFFFFFFULL);
    uint64_t high = (uint64_t)(result >> 64);
    
    printf("Result hash: 0x%016lx%016lx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
