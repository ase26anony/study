/* test_optabs.c - Targeting uncovered lines 8254-8263 in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation that conceptually requires many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern should require expansion with many operands */
    v4si result;
    
    /* Use __builtin_shuffle with multiple vectors and immediate control */
    /* This is designed to potentially trigger case 10: in optabs.cc */
    
    /* First, create intermediate shuffles */
    v4si ab_shuffle = __builtin_shuffle(a, b, (v4si){3, 1, 0, 2});
    v4si cd_shuffle = __builtin_shuffle(c, d, (v4si){1, 3, 2, 0});
    
    /* Complex blend/shuffle with many control elements */
    /* This pattern is designed to require many operands during expansion */
    result = __builtin_shuffle(ab_shuffle, cd_shuffle, (v4si){2, 5, 0, 7});
    
    /* Additional operations to increase operand count */
    result = result + (v4si){1, 2, 3, 4};
    result = result * (v4si){2, 3, 1, 4};
    
    return result;
}

/* Vector permute with many control elements - targets 10+ operand expansion */
static v4sf vector_permute_complex(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Complex permutation that might require many operands */
    v4sf temp1, temp2, result;
    
    /* Multiple shuffle operations with immediate controls */
    temp1 = __builtin_shuffle(v1, v2, (v4si){3, 1, 0, 2});
    temp2 = __builtin_shuffle(v3, v4, (v4si){1, 3, 2, 0});
    
    /* Final shuffle with complex control pattern */
    result = __builtin_shuffle(temp1, temp2, (v4si){2, 5, 0, 7});
    
    /* Add constant operations to increase operand count */
    result = result + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    result = result * (v4sf){2.0f, 3.0f, 1.0f, 4.0f};
    
    return result;
}

/* Custom vector table lookup simulation - designed for many operands */
static v4si vector_table_lookup(v4si table[4], v4si indices) {
    /* Simulate complex table lookup that might expand to many operands */
    v4si result;
    
    /* Extract individual elements for complex lookup */
    int idx0 = indices[0] & 3;
    int idx1 = indices[1] & 3;
    int idx2 = indices[2] & 3;
    int idx3 = indices[3] & 3;
    
    /* Complex element selection - designed to require many operands */
    result[0] = table[idx0][0] + table[idx1][1];
    result[1] = table[idx1][2] * table[idx2][3];
    result[2] = table[idx2][0] - table[idx3][1];
    result[3] = table[idx3][2] ^ table[idx0][3];
    
    return result;
}

/* Complex arithmetic expression with many constants */
static v4sf complex_fma_expression(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Fused multiply-add like expression with many constants */
    v4sf result;
    
    /* Complex expression designed to require many operands during expansion */
    result = a * (v4sf){1.1f, 2.2f, 3.3f, 4.4f} + 
             b * (v4sf){5.5f, 6.6f, 7.7f, 8.8f} + 
             c * (v4sf){9.9f, 10.1f, 11.11f, 12.12f} + 
             d * (v4sf){13.13f, 14.14f, 15.15f, 16.16f};
    
    /* Additional operations */
    result = result + (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    result = result * (v4sf){2.0f, 0.5f, 1.5f, 3.0f};
    
    return result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    volatile int iterations = 3; /* Prevent optimization */
    v4si result_int = {0, 0, 0, 0};
    v4sf result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si a = {argc, argc + 1, argc + 2, argc + 3};
    v4si b = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si c = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si d = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fa = {(float)argc, (float)(argc + 1), (float)(argc + 2), (float)(argc + 3)};
    v4sf fb = {(float)(argc * 2), (float)(argc * 3), (float)(argc * 4), (float)(argc * 5)};
    v4sf fc = {(float)(argc * 6), (float)(argc * 7), (float)(argc * 8), (float)(argc * 9)};
    v4sf fd = {(float)(argc * 10), (float)(argc * 11), (float)(argc * 12), (float)(argc * 13)};
    
    /* Table for lookup operation */
    v4si table[4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    
    /* Loop to prevent dead code elimination */
    for (int i = 0; i < iterations; i++) {
        /* Execute complex operations designed to trigger multi-operand expansion */
        
        /* 1. Complex shuffle operation */
        v4si shuffle_result = complex_shuffle_10_operand(a, b, c, d);
        result_int = result_int + shuffle_result;
        
        /* 2. Complex vector permute */
        v4sf permute_result = vector_permute_complex(fa, fb, fc, fd);
        result_float = result_float + permute_result;
        
        /* 3. Complex FMA expression */
        v4sf fma_result = complex_fma_expression(fa, fb, fc, fd);
        result_float = result_float * fma_result;
        
        /* 4. Table lookup */
        v4si indices = {i, i+1, i+2, i+3};
        v4si lookup_result = vector_table_lookup(table, indices);
        result_int = result_int ^ lookup_result;
        
        /* Modify inputs to prevent optimization */
        a = a + (v4si){1, 2, 3, 4};
        b = b + (v4si){5, 6, 7, 8};
        fa = fa + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
        fb = fb + (v4sf){0.5f, 0.6f, 0.7f, 0.8f};
    }
    
    /* Use results to prevent dead code elimination */
    printf("Integer result: %d %d %d %d\n", 
           result_int[0], result_int[1], result_int[2], result_int[3]);
    printf("Float result: %f %f %f %f\n", 
           result_float[0], result_float[1], result_float[2], result_float[3]);
    
    return 0;
}
