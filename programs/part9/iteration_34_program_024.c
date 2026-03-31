/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation requiring many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d, 
                                        v4si mask1, v4si mask2) {
    /* This complex expression should require many operands during expansion */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){1, 3, 0, 2});
    
    /* Complex blend with multiple constants - may expand to many operands */
    v4si result = __builtin_shuffle(temp1, temp2, mask1);
    
    /* Additional shuffle with immediate control - adds more operands */
    result = __builtin_shuffle(result, mask2);
    
    return result;
}

/* Vector permute with many source elements and control values */
static v4sf vector_permute_10_args(v4sf v0, v4sf v1, v4sf v2, v4sf v3,
                                   v4si ctrl0, v4si ctrl1, v4si ctrl2) {
    /* Multi-step permutation that may require many operands */
    v4sf t0 = __builtin_shuffle(v0, v1, (v4si){0, 2, 1, 3});
    v4sf t1 = __builtin_shuffle(v2, v3, (v4si){1, 3, 0, 2});
    
    /* Complex blend operation - potentially expands to 10 operands */
    v4sf result = __builtin_shuffle(t0, t1, ctrl0);
    
    /* Additional operations with constants */
    result = result + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    result = __builtin_shuffle(result, ctrl1);
    
    return result;
}

/* Fused multiply-add with many constants */
static v4sf fma_many_constants(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Expression with many constants that may expand to many operands */
    v4sf result = a * (v4sf){3.14159f, 2.71828f, 1.41421f, 1.61803f} +
                  b * (v4sf){0.57721f, 1.73205f, 2.23607f, 2.64575f} +
                  c * (v4sf){0.69315f, 1.09861f, 1.60944f, 2.30259f} +
                  d * (v4sf){0.43429f, 0.47712f, 0.49715f, 0.30103f};
    
    return result;
}

/* Complex bitwise operations with many mask constants */
static v4si bitwise_ops_many_masks(v4si x, v4si y, v4si z) {
    /* Series of bitwise ops with distinct constants - may need many operands */
    v4si result = (x & (v4si){0xFF00FF00, 0x00FF00FF, 0xF0F0F0F0, 0x0F0F0F0F}) |
                  (y & (v4si){0xAAAAAAAA, 0x55555555, 0xCCCCCCCC, 0x33333333}) |
                  (z & (v4si){0xF0F0F0F0, 0x0F0F0F0F, 0xFF00FF00, 0x00FF00FF});
    
    result = result ^ (v4si){0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
    
    return result;
}

/* Table lookup simulation with many operands */
static v4si table_lookup_10_operands(v4si indices, v4si table0, v4si table1,
                                     v4si table2, v4si table3) {
    /* Complex expression simulating multi-table lookup */
    v4si mask = (v4si){0x3, 0x3, 0x3, 0x3};  // Mask for 4-entry tables
    
    v4si idx0 = indices & mask;
    v4si idx1 = (indices >> 2) & mask;
    
    /* Shuffles that may expand to many operands */
    v4si result0 = __builtin_shuffle(table0, table1, idx0);
    v4si result1 = __builtin_shuffle(table2, table3, idx1);
    
    v4si final_result = __builtin_shuffle(result0, result1, (v4si){0, 2, 1, 3});
    
    return final_result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    volatile int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 100) iterations = 100;
    }
    
    /* Initialize vectors with non-constant values to prevent optimization */
    v4si a = {argc, argc + 1, argc + 2, argc + 3};
    v4si b = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si c = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si d = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fa = {(float)argc, (float)(argc + 1), (float)(argc + 2), (float)(argc + 3)};
    v4sf fb = {(float)(argc * 2), (float)(argc * 3), (float)(argc * 4), (float)(argc * 5)};
    v4sf fc = {(float)(argc * 6), (float)(argc * 7), (float)(argc * 8), (float)(argc * 9)};
    v4sf fd = {(float)(argc * 10), (float)(argc * 11), (float)(argc * 12), (float)(argc * 13)};
    
    v4si control1 = {0, 2, 1, 3};
    v4si control2 = {1, 3, 0, 2};
    v4si control3 = {2, 0, 3, 1};
    
    v4si table0 = {0x00, 0x11, 0x22, 0x33};
    v4si table1 = {0x44, 0x55, 0x66, 0x77};
    v4si table2 = {0x88, 0x99, 0xAA, 0xBB};
    v4si table3 = {0xCC, 0xDD, 0xEE, 0xFF};
    
    v4si indices = {0, 1, 2, 3};
    
    /* Accumulator to prevent optimization */
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to ensure execution but prevent excessive unrolling */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        v4si shuffle_result = complex_shuffle_10_operands(a, b, c, d, control1, control2);
        int_result = int_result + shuffle_result;
        
        v4sf permute_result = vector_permute_10_args(fa, fb, fc, fd, control1, control2, control3);
        float_result = float_result + permute_result;
        
        v4sf fma_result = fma_many_constants(fa, fb, fc, fd);
        float_result = float_result + fma_result;
        
        v4si bitwise_result = bitwise_ops_many_masks(a, b, c);
        int_result = int_result + bitwise_result;
        
        v4si table_result = table_lookup_10_operands(indices, table0, table1, table2, table3);
        int_result = int_result + table_result;
        
        /* Modify inputs slightly to prevent constant propagation */
        a = a + (v4si){1, 1, 1, 1};
        fa = fa + (v4sf){0.1f, 0.1f, 0.1f, 0.1f};
        indices = indices + (v4si){1, 1, 1, 1};
    }
    
    /* Print results to create side effects */
    printf("Int result: %d %d %d %d\n", 
           int_result[0], int_result[1], int_result[2], int_result[3]);
    printf("Float result: %f %f %f %f\n",
           float_result[0], float_result[1], float_result[2], float_result[3]);
    
    return 0;
}
