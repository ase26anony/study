/* test_optabs.c - Program to trigger 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Complex shuffle operation that conceptually needs many operands */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d, 
                                       int idx0, int idx1, int idx2, int idx3,
                                       int idx4, int idx5) {
    /* This complex shuffle pattern should require many operands during expansion */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){idx0, idx1, idx2, idx3});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){idx4, idx5, idx0+1, idx1+1});
    
    /* Mix with arithmetic operations to prevent simplification */
    v4si result = temp1 + temp2;
    result = result * (v4si){2, 3, 5, 7};  /* Multiply by primes */
    
    /* Another shuffle with immediate indices */
    v4si final = __builtin_shuffle(result, temp1, (v4si){3, 2, 1, 0});
    
    return final;
}

/* Vector blend with many control bits - may expand to multi-operand pattern */
static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d,
                                v4sf weights1, v4sf weights2,
                                float scale1, float scale2, 
                                int blend_mask, int permute_ctrl) {
    /* Complex expression mixing multiple operations */
    v4sf t1 = a * weights1 + b * (v4sf){scale1, scale2, scale1, scale2};
    v4sf t2 = c * weights2 + d * (v4sf){scale2, scale1, scale2, scale1};
    
    /* Conditional blend based on mask - may use many operands */
    v4sf blended;
    for (int i = 0; i < 4; i++) {
        blended[i] = (blend_mask & (1 << i)) ? t1[i] : t2[i];
    }
    
    /* Final permutation */
    v4sf result = __builtin_shuffle(blended, (v4sf){permute_ctrl & 3, 
                                                   (permute_ctrl >> 2) & 3,
                                                   (permute_ctrl >> 4) & 3,
                                                   (permute_ctrl >> 6) & 3});
    
    return result;
}

/* Use __builtin_convertvector with complex type mixing */
static v4si convert_complex_expression(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Complex conversion chain that may require many operands */
    v4sf sum1 = a + b;
    v4sf sum2 = c + d;
    v4sf product = sum1 * sum2;
    
    /* Convert through intermediate types */
    v2di intermediate = __builtin_convertvector(product, v2di);
    
    /* More operations */
    v2di shifted = intermediate >> (v2di){2, 3};
    v4si result = __builtin_convertvector(shifted, v4si);
    
    return result;
}

/* Main test function with non-trivial loop to prevent optimization */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    
    /* Initialize vectors with non-constant values to prevent constant folding */
    v4si vec_a = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec_b = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec_c = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec_d = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf vec_f1 = {argc * 1.0f, argc * 1.5f, argc * 2.0f, argc * 2.5f};
    v4sf vec_f2 = {argc * 3.0f, argc * 3.5f, argc * 4.0f, argc * 4.5f};
    v4sf vec_f3 = {argc * 5.0f, argc * 5.5f, argc * 6.0f, argc * 6.5f};
    v4sf vec_f4 = {argc * 7.0f, argc * 7.5f, argc * 8.0f, argc * 8.5f};
    
    v4sf weights1 = {0.1f, 0.2f, 0.3f, 0.4f};
    v4sf weights2 = {0.5f, 0.6f, 0.7f, 0.8f};
    
    v4si accumulator = {0, 0, 0, 0};
    v4sf float_accumulator = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent dead code elimination */
    for (volatile int i = 0; i < iterations; i++) {
        /* Test 1: Complex shuffle with many operands */
        v4si shuffle_result = complex_shuffle_10_operands(
            vec_a, vec_b, vec_c, vec_d,
            i, i+1, i+2, i+3, i+4, i+5);
        
        /* Test 2: Vector blend with many parameters */
        v4sf blend_result = vector_blend_complex(
            vec_f1, vec_f2, vec_f3, vec_f4,
            weights1, weights2,
            1.5f + i, 2.5f + i,
            0b1010 + i, 0x93 + i);
        
        /* Test 3: Complex conversion chain */
        v4si convert_result = convert_complex_expression(
            vec_f1, vec_f2, vec_f3, vec_f4);
        
        /* Accumulate results to create data dependencies */
        accumulator += shuffle_result + convert_result;
        
        /* Mix float results */
        float_accumulator += blend_result;
        
        /* Modify input vectors slightly to prevent loop invariant removal */
        vec_a[0] += i;
        vec_f1[0] += i * 0.1f;
    }
    
    /* Use results to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += accumulator[i];
        sum += (int)float_accumulator[i];
    }
    
    printf("Result: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
