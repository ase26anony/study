/* Main test file to trigger target hooks for built-in helper generation */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Inline function that uses 128-bit operations - will be included in header */
static inline unsigned __int128 calculate_hash(unsigned __int128 a, unsigned __int128 b) 
    __attribute__((nothrow));

/* Function with nothrow attribute to influence TREE_NOTHROW flag */
static inline unsigned __int128 calculate_hash(unsigned __int128 a, unsigned __int128 b) {
    /* Complex 128-bit arithmetic that may require helper functions */
    unsigned __int128 result = 0;
    
    /* Division/modulo operations often need runtime helpers */
    if (b != 0) {
        result = a / b;        /* May call __udivti3 */
        result += a % b;       /* May call __umodti3 */
    }
    
    return result;
}

/* Atomic operation helper - may generate atomic helper declarations */
unsigned __int128 atomic_update(unsigned __int128 *ptr, unsigned __int128 val) 
    __attribute__((nothrow));

unsigned __int128 atomic_update(unsigned __int128 *ptr, unsigned __int128 val) {
    unsigned __int128 expected, desired;
    unsigned __int128 *expected_ptr = &expected;
    
    /* This atomic operation may require helper functions */
    do {
        expected = *ptr;
        desired = expected + val;
    } while (!__atomic_compare_exchange_n(ptr, &expected, desired, 
                                          0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
    
    return desired;
}

/* OpenMP target region - may generate device helper declarations */
#ifdef _OPENMP
void omp_target_operation(unsigned __int128 *data, int n) 
    __attribute__((nothrow));

void omp_target_operation(unsigned __int128 *data, int n) {
    #pragma omp target teams distribute parallel for map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        /* 128-bit operation inside OpenMP target region */
        data[i] = data[i] * 3 + 1;
    }
}
#endif

int main() {
    volatile unsigned __int128 volatile_var = 0;
    unsigned __int128 regular_var = 100;
    unsigned __int128 divisor = 7;
    unsigned __int128 result = 0;
    
    /* 1. Use volatile 128-bit variable in complex operation */
    volatile_var = ((unsigned __int128)1 << 64) + 12345;
    
    /* 2. Perform 128-bit division - may trigger helper generation */
    regular_var = calculate_hash(volatile_var, divisor);
    
    /* 3. Atomic operations on 128-bit variable */
    unsigned __int128 atomic_var = 0;
    for (int i = 0; i < 10; i++) {
        atomic_update(&atomic_var, regular_var);
    }
    
    /* 4. OpenMP target operation if supported */
    #ifdef _OPENMP
    unsigned __int128 omp_data[4] = {1, 2, 3, 4};
    omp_target_operation(omp_data, 4);
    result += omp_data[0] + omp_data[3];
    #endif
    
    /* 5. More 128-bit arithmetic mixing operations */
    unsigned __int128 a = ((unsigned __int128)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210;
    unsigned __int128 b = ((unsigned __int128)0x1111111111111111 << 64) | 0x2222222222222222;
    
    /* Mix of operations that may need different helpers */
    unsigned __int128 mul_result = a * b;      /* May need __multi3 */
    unsigned __int128 div_result = a / (b + 1); /* May need __divti3 */
    
    result += mul_result + div_result + atomic_var + regular_var;
    
    /* Prevent dead code elimination by printing hash */
    unsigned long long high = (unsigned long long)(result >> 64);
    unsigned long long low = (unsigned long long)result;
    
    printf("Result hash: 0x%016llx%016llx\n", high, low);
    printf("Test completed - check coverage for targhooks.cc lines 981-990\n");
    
    return 0;
}
