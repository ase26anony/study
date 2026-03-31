/* test_optabs.c - Program to trigger 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* Complex shuffle operation that conceptually requires many operands */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern uses multiple operations that may expand
     * to a 10-operand optab entry during RTL expansion */
    
    /* Create a control vector with immediate values - these become operands */
    v4si control1 = {0, 2, 4, 6};
    v4si control2 = {1, 3, 5, 7};
    
    /* Multi-stage shuffle that requires many operands */
    v4si shuffled1 = __builtin_shuffle(a, b, control1);
    v4si shuffled2 = __builtin_shuffle(c, d, control2);
    
    /* Another shuffle combining results with immediate control */
    v4si final_control = {3, 1, 2, 0};
    v4si result = __builtin_shuffle(shuffled1, shuffled2, final_control);
    
    return result;
}

/* Vector blend with many immediate constants */
static v8sf complex_blend_10_operand(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Create blend mask with immediate values */
    v8si blend_mask = {0, -1, 0, -1, -1, 0, -1, 0};
    
    /* Blend a and b using mask */
    v8sf blended1 = __builtin_shuffle(a, b, (v8si){0, 9, 2, 11, 12, 5, 14, 7});
    
    /* Blend c and d using different pattern */
    v8sf blended2 = __builtin_shuffle(c, d, (v8si){8, 1, 10, 3, 4, 13, 6, 15});
    
    /* Final blend of the two blended results */
    v8sf result = __builtin_shuffle(blended1, blended2, (v8si){0, 9, 2, 11, 12, 5, 14, 7});
    
    return result;
}

/* Complex multiply-add pattern with many constants */
static v4sf fma_like_10_operand(v4sf a, v4sf b, v4sf c) {
    /* Fused multiply-add like pattern with multiple constants */
    v4sf result;
    
    /* This complex expression may expand to many operands */
    result = a * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} + 
             b * (v4sf){0.5f, 1.5f, 2.5f, 3.5f} + 
             c * (v4sf){0.25f, 0.75f, 1.25f, 1.75f};
    
    /* Additional shuffle with immediate control */
    v4sf shuffled = __builtin_shuffle(result, (v4sf){0.0f}, (v4si){3, 2, 1, 0});
    
    return shuffled;
}

/* Table lookup simulation that may require many operands */
static v4si table_lookup_10_operand(v4si indices, v4si table0, v4si table1, 
                                    v4si table2, v4si table3) {
    /* Complex table lookup that may expand to multi-operand optab */
    v4si result;
    
    /* Extract individual indices */
    int idx0 = indices[0] & 3;
    int idx1 = indices[1] & 3;
    int idx2 = indices[2] & 3;
    int idx3 = indices[3] & 3;
    
    /* Select from different tables based on index */
    v4si selected0 = __builtin_shuffle(table0, table1, (v4si){idx0, idx0, idx0, idx0});
    v4si selected1 = __builtin_shuffle(table2, table3, (v4si){idx1, idx1, idx1, idx1});
    v4si selected2 = __builtin_shuffle(table0, table1, (v4si){idx2, idx2, idx2, idx2});
    v4si selected3 = __builtin_shuffle(table2, table3, (v4si){idx3, idx3, idx3, idx3});
    
    /* Final combination */
    result = selected0 + selected1 + selected2 + selected3;
    
    return result;
}

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf vec_af = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_bf = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_cf = {9.0f, 10.0f, 11.0f, 12.0f};
    
    v8sf vec_a8 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf vec_b8 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v8sf vec_c8 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f};
    v8sf vec_d8 = {25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Table for lookup */
    v4si table0 = {100, 101, 102, 103};
    v4si table1 = {200, 201, 202, 203};
    v4si table2 = {300, 301, 302, 303};
    v4si table3 = {400, 401, 402, 403};
    v4si indices = {0, 1, 2, 3};
    
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    v8sf float8_result = {0.0f};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        int_result = complex_shuffle_10_operand(vec_a, vec_b, vec_c, vec_d);
        float_result = fma_like_10_operand(vec_af, vec_bf, vec_cf);
        float8_result = complex_blend_10_operand(vec_a8, vec_b8, vec_c8, vec_d8);
        
        /* Modify inputs slightly to prevent constant folding */
        vec_a[0] += i;
        vec_af[0] += (float)i;
        vec_a8[0] += (float)i;
    }
    
    /* Use results to prevent optimization */
    printf("Int result: %d %d %d %d\n", 
           int_result[0], int_result[1], int_result[2], int_result[3]);
    printf("Float result: %.2f %.2f %.2f %.2f\n",
           float_result[0], float_result[1], float_result[2], float_result[3]);
    
    /* Additional complex operation in main to increase chance of hitting target */
    v4si lookup_result = table_lookup_10_operand(indices, table0, table1, table2, table3);
    printf("Lookup result: %d %d %d %d\n",
           lookup_result[0], lookup_result[1], lookup_result[2], lookup_result[3]);
    
    return 0;
}
