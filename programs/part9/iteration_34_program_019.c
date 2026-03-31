/* test_optabs.c - Target coverage for optabs.cc lines 8254-8263 */
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
                                        int idx0, int idx1, int idx2, int idx3,
                                        int idx4, int idx5) {
    /* This should expand to an operation with 10 operands:
       4 vector registers + 6 immediate indices */
    v4si result;
    
    /* Use builtin shufflevector which often expands to multi-operand optabs */
    result = __builtin_shufflevector(a, b, idx0, idx1, idx2, idx3);
    
    /* Mix with another shuffle - compiler may combine into complex operation */
    v4si temp = __builtin_shufflevector(c, d, idx4, idx5, idx0, idx1);
    
    /* Combine results - this creates dependency chain preventing optimization */
    result = result ^ temp;
    
    return result;
}

/* Vector conversion requiring multiple operands */
static v4sf convert_and_shuffle(v2di src1, v2di src2, v4si mask) {
    /* __builtin_convertvector with shuffle may need many operands */
    v4sf converted1 = __builtin_convertvector(src1, v4sf);
    v4sf converted2 = __builtin_convertvector(src2, v4sf);
    
    /* Complex shuffle mixing both converted vectors */
    v4sf result = __builtin_shufflevector(converted1, converted2, 
                                          3, 2, 1, 0);
    
    /* Additional operation to ensure it's not optimized away */
    result = result * (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    
    return result;
}

/* Fused multiply-add pattern with many constants */
static v4sf complex_fma(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Expression with multiple constants that may expand to many operands */
    v4sf result = a * (v4sf){3.14159f, 2.71828f, 1.41421f, 1.61803f} +
                  b * (v4sf){0.57721f, 1.73205f, 2.23607f, 2.64575f} +
                  c * (v4sf){1.0f, 2.0f, 3.0f, 4.0f} +
                  d * (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    /* Additional shuffle to potentially trigger 10-operand case */
    result = __builtin_shufflevector(result, result, 3, 0, 1, 2);
    
    return result;
}

/* Bitwise operations with many mask constants */
static v4si multi_mask_operation(v4si x) {
    /* Series of bitwise ops with distinct constants */
    v4si result = x & (v4si){0xFF00FF00, 0x00FF00FF, 0xFFFF0000, 0x0000FFFF};
    result = result | (v4si){0xAAAAAAAA, 0x55555555, 0xCCCCCCCC, 0x33333333};
    result = result ^ (v4si){0xF0F0F0F0, 0x0F0F0F0F, 0xAAAAAAAA, 0x55555555};
    
    /* Final shuffle with immediate indices */
    result = __builtin_shufflevector(result, result, 2, 3, 0, 1);
    
    return result;
}

/* Main function with non-trivial execution flow */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Initialize vectors with non-constant values to prevent optimization */
    v4si vec1 = {argc, argc + 1, argc + 2, argc + 3};
    v4si vec2 = {argc * 2, argc * 3, argc * 4, argc * 5};
    v4si vec3 = {argc * 6, argc * 7, argc * 8, argc * 9};
    v4si vec4 = {argc * 10, argc * 11, argc * 12, argc * 13};
    
    v4sf fvec1 = {(float)argc, (float)(argc + 1), (float)(argc + 2), (float)(argc + 3)};
    v4sf fvec2 = {(float)(argc * 2), (float)(argc * 3), (float)(argc * 4), (float)(argc * 5)};
    v4sf fvec3 = {(float)(argc * 6), (float)(argc * 7), (float)(argc * 8), (float)(argc * 9)};
    v4sf fvec4 = {(float)(argc * 10), (float)(argc * 11), (float)(argc * 12), (float)(argc * 13)};
    
    v2di dvec1 = {(long long)argc, (long long)(argc + 100)};
    v2di dvec2 = {(long long)(argc * 100), (long long)(argc * 200)};
    
    v4si result_int = {0};
    v4sf result_float = {0.0f};
    
    /* Loop to prevent optimization but keep it small */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call functions that may trigger 10-operand expansions */
        
        /* 1. Complex shuffle with 10 operands (4 vectors + 6 indices) */
        result_int = complex_shuffle_10_operands(vec1, vec2, vec3, vec4,
                                                 i, i+1, i+2, i+3, i+4, i+5);
        
        /* 2. Vector conversion with shuffle */
        result_float = convert_and_shuffle(dvec1, dvec2, (v4si){i, i+1, i+2, i+3});
        
        /* 3. FMA with many constants */
        result_float = complex_fma(fvec1, fvec2, fvec3, fvec4);
        
        /* 4. Bitwise operations with many mask constants */
        result_int = multi_mask_operation(result_int);
        
        /* Mix results to create dependencies */
        fvec1 = fvec1 + __builtin_convertvector(result_int, v4sf);
        vec1 = vec1 + (v4si){(int)result_float[0], (int)result_float[1], 
                             (int)result_float[2], (int)result_float[3]};
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result int: [%d, %d, %d, %d]\n", 
           result_int[0], result_int[1], result_int[2], result_int[3]);
    printf("Result float: [%f, %f, %f, %f]\n",
           result_float[0], result_float[1], result_float[2], result_float[3]);
    
    return (result_int[0] + (int)result_float[0]) != 0 ? 0 : 1;
}
