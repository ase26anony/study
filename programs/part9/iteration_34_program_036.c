/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation using multiple vectors and immediate controls */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * - 4 source vectors (a, b, c, d)
     * - 6 immediate control values for shuffle indices
     * Total: 10 operands
     */
    
    /* Use built-in shuffle with multiple vectors and complex control */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 4, 2, 6});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){1, 5, 3, 7});
    
    /* Another layer of shuffle mixing results */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){0, 2, 4, 6});
    
    /* Additional operations to ensure complex expansion */
    result = result + __builtin_shuffle(a, c, (v4si){3, 1, 7, 5});
    result = result * __builtin_shuffle(b, d, (v4si){2, 6, 0, 4});
    
    return result;
}

/* Vector reduction with multiple constants - may expand to many operands */
static v4sf complex_fma_pattern(v4sf x, v4sf y, v4sf z, v4sf w) {
    /* Fused multiply-add pattern with multiple constants */
    v4sf result = x * (v4sf){1.414f, 2.718f, 3.142f, 1.618f} +
                  y * (v4sf){0.577f, 1.732f, 2.236f, 3.333f} +
                  z * (v4sf){1.234f, 5.678f, 9.012f, 3.456f} +
                  w * (v4sf){7.890f, 2.468f, 1.357f, 9.246f};
    
    /* Additional shuffle to potentially trigger 10-operand expansion */
    v4sf shuffled = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    return result + shuffled * 0.5f;
}

/* Atomic-like operation simulation with many parameters */
static v2di multi_operand_atomic_pattern(v2di mem, v2di val, v2di mask) {
    /* Complex bitwise operation that might expand to many operands */
    v2di result = (mem & mask) | (val & ~mask);
    
    /* Additional operations with constants */
    result = result ^ (v2di){0xFFFFFFFFFFFFFFFFULL, 0xAAAAAAAAAAAAAAAALL};
    result = result + (v2di){0x5555555555555555ULL, 0xCCCCCCCCCCCCCCCCULL};
    
    /* Shuffle between different vector types */
    v4si temp = __builtin_convertvector(result, v4si);
    temp = __builtin_shuffle(temp, temp, (v4si){1, 0, 3, 2});
    
    return __builtin_convertvector(temp, v2di);
}

/* Main test function with loops to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with pattern */
    v4si vec_a = (v4si){1, 2, 3, 4};
    v4si vec_b = (v4si){5, 6, 7, 8};
    v4si vec_c = (v4si){9, 10, 11, 12};
    v4si vec_d = (v4si){13, 14, 15, 16};
    
    v4sf vec_x = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_y = (v4sf){5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_z = (v4sf){9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_w = (v4sf){13.0f, 14.0f, 15.0f, 16.0f};
    
    v2di vec_mem = (v2di){0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL};
    v2di vec_val = (v2di){0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL};
    v2di vec_mask = (v2di){0xFFFFFFFF00000000ULL, 0x00000000FFFFFFFFULL};
    
    v4si int_result = {0};
    v4sf float_result = {0};
    v2di long_result = {0};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        int_result = complex_shuffle_10_operand(vec_a, vec_b, vec_c, vec_d);
        float_result = complex_fma_pattern(vec_x, vec_y, vec_z, vec_w);
        long_result = multi_operand_atomic_pattern(vec_mem, vec_val, vec_mask);
        
        /* Modify inputs slightly to prevent constant propagation */
        vec_a += (v4si){i, i, i, i};
        vec_x += (v4sf){i * 0.1f, i * 0.1f, i * 0.1f, i * 0.1f};
        vec_mem += (v2di){i, i};
    }
    
    /* Use results to prevent elimination */
    int sum = int_result[0] + int_result[1] + int_result[2] + int_result[3];
    float fsum = float_result[0] + float_result[1] + float_result[2] + float_result[3];
    long long lsum = long_result[0] + long_result[1];
    
    printf("Results: int_sum=%d, float_sum=%.2f, long_sum=%lld\n", 
           sum, fsum, lsum);
    
    return (sum > 0) ? 0 : 1;
}
