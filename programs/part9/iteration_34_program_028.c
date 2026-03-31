/* test_optabs.c - Program to trigger 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle operation using multiple vectors and immediate controls */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This should expand to something requiring 10 operands:
     * 4 source vectors + 6 immediate control values
     */
    v4si result;
    
    /* Use __builtin_shuffle with multiple vectors and complex control */
    /* First, create intermediate shuffles */
    v4si ab_shuffle = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    v4si cd_shuffle = __builtin_shuffle(c, d, (v4si){2, 6, 3, 7});
    
    /* Complex shuffle combining all 4 vectors with immediate control */
    /* This pattern uses 6 immediate indices to select from 4 vectors */
    result = __builtin_shufflevector(ab_shuffle, cd_shuffle, 
                                     3, 7, 1, 5);  /* 4 operands + 4 indices = 8 total */
    
    /* Additional operation that might require more operands */
    result = result + (v4si){1, 2, 3, 4};  /* Adds 4 more constants */
    
    return result;
}

/* Vector blend with many constants */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex blend operation that might require many operands */
    v4sf temp1 = __builtin_shuffle(a, b, (v4si){0, 1, 4, 5});
    v4sf temp2 = __builtin_shuffle(c, d, (v4si){2, 3, 6, 7});
    
    /* Blend with immediate mask - each bit selects between temp1 and temp2 */
    /* This could expand to an operation with many operands */
    v4sf result = __builtin_ia32_blendps(temp1, temp2, 0x5);  /* 0101 binary */
    
    /* Additional operations with constants */
    result = result * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} + 
             (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    return result;
}

/* Fused multiply-add with many constants */
static float complex_fma_chain(float a, float b, float c, float d) {
    /* Chain of FMA operations with many constants */
    float result = __builtin_fmaf(a, 1.1f, 0.1f);
    result = __builtin_fmaf(b, 2.2f, result);
    result = __builtin_fmaf(c, 3.3f, result);
    result = __builtin_fmaf(d, 4.4f, result);
    result = __builtin_fmaf(result, 5.5f, 6.6f);
    
    return result;
}

/* Table lookup simulation with many operands */
static v4si vector_table_lookup(v4si table0, v4si table1, 
                                v4si table2, v4si table3,
                                v4si indices) {
    /* Simulate a 4-table lookup - might expand to complex operation */
    v4si mask = (v4si){0x3, 0x3, 0x3, 0x3};
    v4si table_select = indices & mask;
    v4si lane_select = indices >> 2;
    
    /* This complex selection might require many operands during expansion */
    v4si result;
    
    /* Manual blend based on table_select - each element could come from different table */
    for (int i = 0; i < 4; i++) {
        int table = table_select[i] & 0x3;
        int lane = lane_select[i] & 0x3;
        
        switch (table) {
            case 0: result[i] = table0[lane]; break;
            case 1: result[i] = table1[lane]; break;
            case 2: result[i] = table2[lane]; break;
            case 3: result[i] = table3[lane]; break;
        }
    }
    
    return result;
}

/* Main function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations > 100) iterations = 100;
    }
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec_a = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec_c = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec_d = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf vec_af = {(float)argc, (float)(argc + 1), (float)(argc + 2), (float)(argc + 3)};
    v4sf vec_bf = {(float)(argc * 2), (float)(argc * 3), (float)(argc * 4), (float)(argc * 5)};
    v4sf vec_cf = {(float)(argc * 6), (float)(argc * 7), (float)(argc * 8), (float)(argc * 9)};
    v4sf vec_df = {(float)(argc * 10), (float)(argc * 11), (float)(argc * 12), (float)(argc * 13)};
    
    v4si result_int = {0, 0, 0, 0};
    v4sf result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    float scalar_result = 0.0f;
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that might trigger 10-operand expansions */
        result_int = complex_shuffle_10_operands(vec_a, vec_b, vec_c, vec_d);
        result_float = vector_blend_complex(vec_af, vec_bf, vec_cf, vec_df);
        scalar_result = complex_fma_chain((float)argc, (float)(argc + 1), 
                                         (float)(argc + 2), (float)(argc + 3));
        
        /* Table lookup with many vectors */
        v4si indices = {i, i + 1, i + 2, i + 3};
        v4si table_result = vector_table_lookup(vec_a, vec_b, vec_c, vec_d, indices);
        result_int = result_int + table_result;
    }
    
    /* Use results to prevent optimization */
    printf("Results: %d %d %d %d\n", 
           result_int[0], result_int[1], result_int[2], result_int[3]);
    printf("Float results: %f %f %f %f\n",
           result_float[0], result_float[1], result_float[2], result_float[3]);
    printf("Scalar result: %f\n", scalar_result);
    
    return 0;
}
