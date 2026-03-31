/* test_optabs.c - Program to trigger 10-operand optab expansion */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation using multiple vectors and immediate controls */
static v4si complex_shuffle_10_operand(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually requires:
     * 4 source vectors (a, b, c, d)
     * 6 immediate control values (shuffle indices)
     * Total: 10 operands
     */
    
    /* Create a complex shuffle pattern using builtins */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){2, 6, 3, 7});
    
    /* Final shuffle combining results - this may expand to a 10-operand pattern */
    v4si result = __builtin_shuffle(temp1, temp2, (v4si){0, 2, 4, 6});
    
    return result;
}

/* Vector blend with multiple control masks */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex blending operation that might require many operands */
    v4sf mask1 = (v4sf){0.0f, 1.0f, 0.0f, 1.0f};
    v4sf mask2 = (v4sf){1.0f, 0.0f, 1.0f, 0.0f};
    
    /* Complex expression that may expand to multi-operand pattern */
    v4sf temp = __builtin_shuffle(a, b, (v4si){0, 4, 2, 6});
    v4sf temp2 = __builtin_shuffle(c, d, (v4si){1, 5, 3, 7});
    
    /* Conditional blend based on masks - this is complex enough
     * that it might require a 10-operand expansion */
    v4sf result = temp * mask1 + temp2 * mask2;
    
    return result;
}

/* Multi-operand vector conversion and shuffle */
static v2df complex_vector_conversion(v4si int_vec, v4sf float_vec) {
    /* Convert and shuffle between different vector types */
    v2di int64_vec = __builtin_convertvector(int_vec, v2di);
    v2df double_vec = __builtin_convertvector(float_vec, v2df);
    
    /* Complex shuffle pattern that might require many operands */
    v2df shuffled = __builtin_shuffle(double_vec, double_vec, (v2di){1, 0});
    
    /* Mix with converted integer vector */
    v2df result = shuffled + __builtin_convertvector(int64_vec, v2df);
    
    return result;
}

/* Main test function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations > 100) iterations = 100;
    }
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec_a = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = {argc + 4, argc + 5, argc + 6, argc + 7};
    v4si vec_c = {argc + 8, argc + 9, argc + 10, argc + 11};
    v4si vec_d = {argc + 12, argc + 13, argc + 14, argc + 15};
    
    v4sf vec_fa = {(float)argc, (float)(argc + 1), (float)(argc + 2), (float)(argc + 3)};
    v4sf vec_fb = {(float)(argc + 4), (float)(argc + 5), (float)(argc + 6), (float)(argc + 7)};
    v4sf vec_fc = {(float)(argc + 8), (float)(argc + 9), (float)(argc + 10), (float)(argc + 11)};
    v4sf vec_fd = {(float)(argc + 12), (float)(argc + 13), (float)(argc + 14), (float)(argc + 15)};
    
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    v2df double_result = {0.0, 0.0};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        int_result = complex_shuffle_10_operand(vec_a, vec_b, vec_c, vec_d);
        float_result = vector_blend_complex(vec_fa, vec_fb, vec_fc, vec_fd);
        double_result = complex_vector_conversion(vec_a, vec_fa);
        
        /* Mix results to create data dependencies */
        vec_a = vec_a + int_result;
        vec_fa = vec_fa + float_result;
    }
    
    /* Use results to prevent optimization */
    int sum = int_result[0] + int_result[1] + int_result[2] + int_result[3];
    float fsum = float_result[0] + float_result[1] + float_result[2] + float_result[3];
    double dsum = double_result[0] + double_result[1];
    
    printf("Results: %d, %f, %f\n", sum, fsum, dsum);
    
    return sum > 0 ? 0 : 1;
}

/* Additional complex expression that might trigger multi-operand expansion */
v4si __attribute__((noinline)) 
complex_multi_operand_expr(v4si a, v4si b, v4si c, v4si d, 
                          v4si e, v4si f, v4si g, v4si h) {
    /* This complex expression uses many vectors and may require
     * a 10-operand expansion during optimization */
    
    /* Multi-step shuffle and blend */
    v4si t1 = __builtin_shuffle(a, b, (v4si){0, 4, 2, 6});
    v4si t2 = __builtin_shuffle(c, d, (v4si){1, 5, 3, 7});
    v4si t3 = __builtin_shuffle(e, f, (v4si){2, 6, 0, 4});
    v4si t4 = __builtin_shuffle(g, h, (v4si){3, 7, 1, 5});
    
    /* Complex combination - this might be expanded as a single
     * operation with many operands */
    v4si result = (t1 & t2) | (t3 & t4);
    result = result ^ (v4si){0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    
    return result;
}
