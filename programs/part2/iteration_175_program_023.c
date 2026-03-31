/* Test program to trigger target hook helper generation with specific flags */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Force generation of 128-bit helpers */
#ifdef __cplusplus
extern "C" {
#endif

/* Inline function that uses 128-bit operations - will be included in multiple TUs */
static inline unsigned __int128 calculate_hash(unsigned __int128 a, unsigned __int128 b) {
    /* Complex 128-bit operations that may require runtime helpers */
    unsigned __int128 result = a / b;          /* May call __udivti3 */
    result += a % (b + 1);                     /* May call __umodti3 */
    result *= 1234567890123456789ULL;          /* May call __multi3 */
    return result;
}

/* Function with nothrow attribute */
int __attribute__((nothrow)) atomic_update(volatile __int128 *ptr, __int128 val) {
    __int128 expected, desired;
    int success;
    
    do {
        expected = *ptr;
        desired = expected + val;
        /* Atomic compare-exchange on 128-bit - may require helper */
        success = __atomic_compare_exchange_n(ptr, &expected, desired, 
                                              0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    } while (!success);
    
    return 0;
}

#ifdef __cplusplus
}
#endif

/* OpenMP target region (if supported) */
#ifdef _OPENMP
void omp_target_operation(__int128 *data, int n) {
    #pragma omp target map(tofrom: data[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            /* Use 128-bit operations inside target region */
            __int128 temp = data[i];
            temp = temp / 1000;  /* May trigger helper generation */
            data[i] = temp;
        }
    }
}
#endif

/* OpenACC parallel region (if supported) */
#ifdef _OPENACC
void acc_parallel_operation(__int128 *data, int n) {
    #pragma acc parallel loop copy(data[0:n])
    for (int i = 0; i < n; i++) {
        __int128 val = data[i];
        val = val % 1000000007;  /* May trigger helper generation */
        data[i] = val;
    }
}
#endif

int main() {
    volatile __int128 volatile_var = 0;
    __int128 regular_var = 1234567890123456789ULL;
    __int128 divisor = 987654321ULL;
    
    /* Force usage of volatile 128-bit variable */
    volatile_var = regular_var;
    
    /* Perform 128-bit division that may require helper */
    __int128 quotient = regular_var / divisor;
    
    /* Perform 128-bit modulo that may require helper */
    __int128 remainder = regular_var % (divisor + 1);
    
    /* Atomic operations on 128-bit */
    __int128 atomic_var = 0;
    __atomic_store_n(&atomic_var, quotient, __ATOMIC_RELAXED);
    
    /* Atomic compare-exchange */
    atomic_update(&atomic_var, remainder);
    
    /* Calculate using inline function */
    unsigned __int128 hash = calculate_hash((unsigned __int128)regular_var, 
                                           (unsigned __int128)divisor);
    
    /* Use OpenMP if available */
    #ifdef _OPENMP
    {
        __int128 omp_data[4] = {regular_var, quotient, remainder, atomic_var};
        #pragma omp parallel
        {
            /* Simple parallel operation */
            #pragma omp for
            for (int i = 0; i < 4; i++) {
                omp_data[i] = omp_data[i] / 1000;
            }
        }
        
        #ifdef _OPENMP
        if (__OPENMP >= 201511) {
            /* Try to trigger target hooks with OpenMP target */
            omp_target_operation(omp_data, 4);
        }
        #endif
    }
    #endif
    
    /* Use OpenACC if available */
    #ifdef _OPENACC
    {
        __int128 acc_data[4] = {regular_var, quotient, remainder, atomic_var};
        acc_parallel_operation(acc_data, 4);
        hash += (unsigned __int128)acc_data[0];
    }
    #endif
    
    /* Prevent dead code elimination */
    uint64_t hash_low = (uint64_t)(hash & 0xFFFFFFFFFFFFFFFFULL);
    uint64_t hash_high = (uint64_t)(hash >> 64);
    
    /* Mix results to ensure all operations are used */
    uint64_t final_hash = hash_low ^ hash_high ^ 
                         (uint64_t)(quotient & 0xFFFFFFFFFFFFFFFFULL) ^
                         (uint64_t)(remainder & 0xFFFFFFFFFFFFFFFFULL) ^
                         (uint64_t)(atomic_var & 0xFFFFFFFFFFFFFFFFULL);
    
    printf("Result hash: 0x%016llx\n", (unsigned long long)final_hash);
    
    return (final_hash != 0) ? 0 : 1;
}
