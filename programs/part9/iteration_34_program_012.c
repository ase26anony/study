/* test_optabs.c - Targeting case 10: 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation requiring many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * - 4 source vectors (a, b, c, d)
     * - 6 immediate control values (shuffle indices)
     * Total: 10 operands
     */
    
    /* Create intermediate shuffles */
    v4si ab_shuffle = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si cd_shuffle = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Complex blend with immediate mask */
    v4si result = __builtin_shuffle(ab_shuffle, cd_shuffle, (v4si){0, 5, 2, 7});
    
    /* Additional permutation with immediate control */
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    return result;
}

/* Vector conversion with many operands */
static v4sf vector_conversion_complex(v4si a, v4si b, v4si c, v4si d) {
    /* Convert vectors with complex pattern */
    v4sf fa = __builtin_convertvector(a, v4sf);
    v4sf fb = __builtin_convertvector(b, v4sf);
    v4sf fc = __builtin_convertvector(c, v4sf);
    v4sf fd = __builtin_convertvector(d, v4sf);
    
    /* Complex shuffle of converted vectors */
    v4sf shuffle1 = __builtin_shuffle(fa, fb, (v4si){0, 5, 2, 7});
    v4sf shuffle2 = __builtin_shuffle(fc, fd, (v4si){4, 1, 6, 3});
    
    /* Final blend - this may expand to multi-operand operation */
    v4sf result = __builtin_shuffle(shuffle1, shuffle2, (v4si){0, 5, 2, 7});
    
    return result;
}

/* Multi-operand arithmetic expression */
static v4sf complex_fma_expression(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Fused multiply-add with multiple constants
     * Each operation may expand to multiple operands
     */
    v4sf result;
    
    /* Complex expression that might be expanded as a single operation
     * with many operands (constants + vectors) */
    result = a * (v4sf){1.1f, 2.2f, 3.3f, 4.4f} + 
             b * (v4sf){5.5f, 6.6f, 7.7f, 8.8f} + 
             c * (v4sf){9.9f, 10.1f, 11.11f, 12.12f} + 
             d * (v4sf){13.13f, 14.14f, 15.15f, 16.16f};
    
    /* Additional shuffle to potentially trigger complex expansion */
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    return result;
}

/* Table lookup simulation with many operands */
static v4si vector_table_lookup(v4si a, v4si b, v4si c, v4si d) {
    /* Simulate a table lookup operation that might require many operands
     * This uses multiple shuffles with immediate indices */
    
    /* First level shuffle - interleave a and b */
    v4si ab = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    
    /* Second level shuffle - interleave c and d */
    v4si cd = __builtin_shuffle(c, d, (v4si){2, 6, 3, 7});
    
    /* Third level - blend ab and cd with pattern */
    v4si result = __builtin_shuffle(ab, cd, (v4si){0, 5, 2, 7});
    
    /* Final permutation */
    result = __builtin_shuffle(result, result, (v4si){3, 0, 1, 2});
    
    return result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    volatile int iterations = 3; /* Prevent optimization */
    if (argc > 1) {
        iterations = atoi(argv[1]) % 5;
        if (iterations < 1) iterations = 1;
    }
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec1 = {argc * 1.1f, argc * 1.2f, argc * 1.3f, argc * 1.4f};
    v4sf fvec2 = {argc * 1.5f, argc * 1.6f, argc * 1.7f, argc * 1.8f};
    v4sf fvec3 = {argc * 1.9f, argc * 2.0f, argc * 2.1f, argc * 2.2f};
    v4sf fvec4 = {argc * 2.3f, argc * 2.4f, argc * 2.5f, argc * 2.6f};
    
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        int_result = complex_shuffle_10_operands(vec1, vec2, vec3, vec4);
        
        float_result = vector_conversion_complex(vec1, vec2, vec3, vec4);
        
        v4sf fma_result = complex_fma_expression(fvec1, fvec2, fvec3, fvec4);
        
        v4si table_result = vector_table_lookup(vec1, vec2, vec3, vec4);
        
        /* Mix results to create data dependencies */
        int v = int_result[0] + table_result[0];
        float f = float_result[0] + fma_result[0];
        
        /* Use results to prevent optimization */
        if (v > 1000 || f > 1000.0f) {
            printf("Unexpected large value: %d, %f\n", v, f);
        }
    }
    
    /* Print final results to ensure side effects */
    printf("Results: [%d, %d, %d, %d]\n", 
           int_result[0], int_result[1], int_result[2], int_result[3]);
    printf("Float results: [%f, %f, %f, %f]\n",
           float_result[0], float_result[1], float_result[2], float_result[3]);
    
    return 0;
}
