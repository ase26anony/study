/* test_optabs_10_operands.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle with many operands - designed to potentially trigger 10-operand expansion */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d, 
                                        int idx0, int idx1, int idx2, int idx3,
                                        int idx4, int idx5) {
    /* This complex expression may expand to an optab requiring 10 operands:
       - 4 source vectors (a, b, c, d) = 4 operands
       - 6 immediate indices = 6 operands
       Total: 10 operands
    */
    
    /* Create a shuffle mask from the 6 indices - use modulo to ensure valid indices */
    int mask[4];
    mask[0] = idx0 % 8;
    mask[1] = idx1 % 8;
    mask[2] = idx2 % 8;
    mask[3] = idx3 % 8;
    
    /* Combine all vectors into an array for selection */
    int combined[16];
    memcpy(&combined[0], &a, sizeof(v4si));
    memcpy(&combined[4], &b, sizeof(v4si));
    memcpy(&combined[8], &c, sizeof(v4si));
    memcpy(&combined[12], &d, sizeof(v4si));
    
    /* Select elements based on mask - complex pattern that might use table lookup */
    v4si result;
    for (int i = 0; i < 4; i++) {
        ((int*)&result)[i] = combined[mask[i]];
    }
    
    /* Additional operation using idx4 and idx5 to ensure all 10 "operands" are used */
    if (idx4 > idx5) {
        result = result + (idx4 - idx5);
    } else {
        result = result + (idx5 - idx4);
    }
    
    return result;
}

/* Vector permute with immediate control - may use __builtin_shuffle with many args */
static v4sf vector_permute_10_args(v4sf v0, v4sf v1, v4sf v2, v4sf v3) {
    /* Complex shuffle pattern that might require many operands during expansion */
    v4sf temp1 = __builtin_shuffle(v0, v1, (v4si){3, 2, 1, 0});
    v4sf temp2 = __builtin_shuffle(v2, v3, (v4si){1, 0, 3, 2});
    
    /* Nested shuffles with immediate controls - each immediate is a potential operand */
    v4sf result = __builtin_shuffle(temp1, temp2, (v4si){
        (int)v0[0] % 4,  /* Use vector elements to create dynamic but compile-time known indices */
        (int)v1[1] % 4,
        (int)v2[2] % 4,
        (int)v3[3] % 4
    });
    
    return result;
}

/* Complex multiply-add pattern with many constants */
static v4sf complex_fma_pattern(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* FMA with multiple constants - may expand to many operands */
    v4sf result = a * (v4sf){3.14159f, 2.71828f, 1.41421f, 1.61803f} +
                  b * (v4sf){0.57721f, 1.73205f, 2.23607f, 2.64575f} +
                  c * (v4sf){0.70710f, 1.22474f, 1.58113f, 1.87082f} +
                  d * (v4sf){0.86602f, 1.11803f, 1.32287f, 1.5f};
    
    /* Additional shuffle to potentially trigger complex expansion */
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    return result;
}

/* Use GCC built-in with many arguments - targeting vector blend */
static v4si vector_blend_complex(v4si a, v4si b, v4si c, v4si d,
                                 int mask0, int mask1, int mask2, int mask3,
                                 int blend_ctrl) {
    /* Complex blend operation that might require many operands */
    v4si ab_blend = __builtin_shuffle(a, b, (v4si){
        mask0 ? 4 : 0,  /* Select from b if mask0 != 0, else from a */
        mask1 ? 5 : 1,
        mask2 ? 6 : 2,
        mask3 ? 7 : 3
    });
    
    v4si cd_blend = __builtin_shuffle(c, d, (v4si){
        (!mask0) ? 8 : 12,  /* Select from c or d based on inverted mask */
        (!mask1) ? 9 : 13,
        (!mask2) ? 10 : 14,
        (!mask3) ? 11 : 15
    });
    
    /* Final blend based on blend_ctrl */
    v4si result;
    for (int i = 0; i < 4; i++) {
        ((int*)&result)[i] = (blend_ctrl & (1 << i)) ? 
                            ((int*)&ab_blend)[i] : ((int*)&cd_blend)[i];
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;
    
    /* Initialize vectors with different patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v4sf vec_fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_fc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_fd = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v4si final_result_int = {0, 0, 0, 0};
    v4sf final_result_float = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent dead code elimination but not trigger excessive unrolling */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        
        /* 1. Complex shuffle with many parameters */
        v4si shuffle_result = complex_shuffle_10_operands(
            vec_a + i, vec_b + i, vec_c + i, vec_d + i,
            i * 2, i * 3, i * 5, i * 7, i * 11, i * 13
        );
        final_result_int = final_result_int + shuffle_result;
        
        /* 2. Vector permute with immediate controls */
        v4sf permute_result = vector_permute_10_args(
            vec_fa + (float)i,
            vec_fb + (float)i,
            vec_fc + (float)i,
            vec_fd + (float)i
        );
        final_result_float = final_result_float + permute_result;
        
        /* 3. Complex FMA pattern with many constants */
        v4sf fma_result = complex_fma_pattern(
            vec_fa * (float)(i + 1),
            vec_fb * (float)(i + 2),
            vec_fc * (float)(i + 3),
            vec_fd * (float)(i + 4)
        );
        final_result_float = final_result_float * 0.5f + fma_result * 0.5f;
        
        /* 4. Vector blend with many control parameters */
        v4si blend_result = vector_blend_complex(
            vec_a * (i + 1),
            vec_b * (i + 2),
            vec_c * (i + 3),
            vec_d * (i + 4),
            i & 1, (i >> 1) & 1, (i >> 2) & 1, (i >> 3) & 1,
            i * 0x1111
        );
        final_result_int = final_result_int - blend_result;
    }
    
    /* Use results to prevent dead code elimination */
    int sum_int = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        sum_int += ((int*)&final_result_int)[i];
        sum_float += ((float*)&final_result_float)[i];
    }
    
    printf("Result checksum - int: %d, float: %f\n", sum_int, sum_float);
    
    return (sum_int > 0 && sum_float > 0.0f) ? 0 : 1;
}
