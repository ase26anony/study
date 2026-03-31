/* test_optabs_10_operands.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation requiring many operands */
static inline v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d,
                                               int idx0, int idx1, int idx2,
                                               int idx3, int idx4, int idx5) {
    /* This complex shuffle pattern conceptually uses:
     * 4 source vectors (a, b, c, d) = 4 operands
     * 6 immediate indices = 6 operands
     * Total: 10 operands
     * 
     * The compiler may expand this into an internal function
     * that requires all 10 values during RTL expansion
     */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){idx0, idx1, 2, 3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){0, 1, idx2, idx3});
    
    /* Final shuffle combining results with more immediate indices */
    return __builtin_shuffle(temp1, temp2, (v4si){idx4, 5, idx5, 7});
}

/* Vector blend with many control bits */
static inline v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d,
                                        int mask0, int mask1, int mask2,
                                        int mask3, float scale, float bias) {
    /* Complex blending operation that might expand to many operands */
    v4sf t1 = a * (v4sf){scale, 1.0f/scale, bias, 1.0f/bias};
    v4sf t2 = b * (v4sf){bias, scale, 1.0f/bias, 1.0f/scale};
    
    /* Conditional blend based on mask bits */
    v4sf result = __builtin_shuffle(t1, t2, 
        (v4si){mask0 ? 0 : 4, mask1 ? 1 : 5, mask2 ? 2 : 6, mask3 ? 3 : 7});
    
    /* Additional operation with c and d */
    result = result + __builtin_shuffle(c, d, 
        (v4si){mask3, mask2, mask1, mask0});
    
    return result;
}

/* Multi-operand vector conversion pattern */
static inline v2di complex_conversion(v4si a, v4si b, v2df c, v2df d,
                                      int shift, int saturate, 
                                      double scale, double offset,
                                      int round_mode, int sign) {
    /* Complex conversion that might require many operands during expansion */
    v2di temp1 = __builtin_convertvector(a + b, v2di);
    v2di temp2 = __builtin_convertvector(
        __builtin_shuffle(a, b, (v4si){3, 2, 1, 0}), v2di);
    
    /* Apply various transformations */
    if (shift > 0) {
        temp1 = temp1 << shift;
        temp2 = temp2 >> (32 - shift);
    }
    
    if (saturate) {
        /* Simulate saturation logic */
        v2di max_val = (v2di){0x7FFFFFFFFFFFFFFFLL, 0x7FFFFFFFFFFFFFFFLL};
        temp1 = temp1 > max_val ? max_val : temp1;
        temp2 = temp2 > max_val ? max_val : temp2;
    }
    
    /* Combine with floating-point vectors */
    v2di conv_c = __builtin_convertvector(c * scale + offset, v2di);
    v2di conv_d = __builtin_convertvector(d * offset + scale, v2di);
    
    /* Final shuffle with immediate indices */
    return __builtin_shuffle(temp1, temp2, (v2di){round_mode, sign ? 1 : 0}) +
           __builtin_shuffle(conv_c, conv_d, (v2di){sign, round_mode});
}

/* Main test function */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with non-constant values to prevent optimization */
    v4si vec_a = (v4si){argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = (v4si){argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec_c = (v4si){argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec_d = (v4si){argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf vec_f1 = (v4sf){argc * 1.0f, argc * 1.5f, argc * 2.0f, argc * 2.5f};
    v4sf vec_f2 = (v4sf){argc * 3.0f, argc * 3.5f, argc * 4.0f, argc * 4.5f};
    v4sf vec_f3 = (v4sf){argc * 5.0f, argc * 5.5f, argc * 6.0f, argc * 6.5f};
    v4sf vec_f4 = (v4sf){argc * 7.0f, argc * 7.5f, argc * 8.0f, argc * 8.5f};
    
    v2df vec_d1 = (v2df){argc * 1.0, argc * 1.1};
    v2df vec_d2 = (v2df){argc * 1.2, argc * 1.3};
    
    v4si final_result_int = {0};
    v4sf final_result_float = {0};
    v2di final_result_long = {0};
    
    /* Loop to prevent optimization but keep it small */
    for (volatile int i = 0; i < iterations; i++) {
        /* Test 1: Integer shuffle with 10 conceptual operands */
        final_result_int = complex_shuffle_10_operands(
            vec_a, vec_b, vec_c, vec_d,
            i & 3, (i + 1) & 3, (i + 2) & 3,
            (i + 3) & 3, (i + 4) & 3, (i + 5) & 3);
        
        /* Test 2: Float blend with many parameters */
        final_result_float = vector_blend_complex(
            vec_f1, vec_f2, vec_f3, vec_f4,
            i & 1, (i >> 1) & 1, (i >> 2) & 1, (i >> 3) & 1,
            1.0f + i * 0.1f, 2.0f + i * 0.2f);
        
        /* Test 3: Complex conversion with many operands */
        final_result_long = complex_conversion(
            vec_a, vec_b, vec_d1, vec_d2,
            i & 7, i & 1, 1.0 + i * 0.01, 0.5 + i * 0.02,
            i & 3, i & 1);
        
        /* Mix results to create data dependencies */
        vec_a = vec_a + final_result_int;
        vec_f1 = vec_f1 + final_result_float;
    }
    
    /* Use results to prevent dead code elimination */
    int sum_int = final_result_int[0] + final_result_int[1] + 
                  final_result_int[2] + final_result_int[3];
    
    float sum_float = final_result_float[0] + final_result_float[1] + 
                      final_result_float[2] + final_result_float[3];
    
    long long sum_long = final_result_long[0] + final_result_long[1];
    
    printf("Results: int=%d, float=%.2f, long=%lld\n", 
           sum_int, sum_float, sum_long);
    
    return (sum_int + (int)sum_float + (int)sum_long) & 0xFF;
}
