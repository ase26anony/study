/* test_optabs.c - Program to trigger 10-operand optab expansion in GCC */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* Complex shuffle operation using multiple vectors and immediate controls */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This should generate a pattern requiring 10 operands:
     * 4 source vectors + 6 immediate control values
     */
    v4si result;
    
    /* Use __builtin_shuffle with complex pattern that might expand to 10 operands */
    /* First, create intermediate shuffles */
    v4si ab_shuffle = __builtin_shuffle(a, b, (v4si){3, 1, 0, 2});
    v4si cd_shuffle = __builtin_shuffle(c, d, (v4si){1, 3, 2, 0});
    
    /* Complex shuffle combining all 4 vectors with many immediate controls */
    /* This pattern uses multiple shuffles that might be combined into one operation */
    result = __builtin_shuffle(ab_shuffle, cd_shuffle, (v4si){2, 5, 0, 7});
    
    /* Additional operations to create complex expression tree */
    result = result + __builtin_shuffle(a, c, (v4si){0, 2, 1, 3});
    result = result * __builtin_shuffle(b, d, (v4si){3, 1, 2, 0});
    
    return result;
}

/* Vector blend operation with many constants */
static v8sf vector_blend_complex(v8sf a, v8sf b, v8sf c, v8sf d) {
    /* Complex blend pattern with many immediate control bits */
    v8sf result;
    
    /* Create a complex blend that might require 10 operands:
     * 4 source vectors + blend control mask (8 bits) + other parameters
     */
    v8sf ab_blend = __builtin_shuffle(a, b, (v8si){0, 9, 2, 11, 4, 13, 6, 15});
    v8sf cd_blend = __builtin_shuffle(c, d, (v8si){8, 1, 10, 3, 12, 5, 14, 7});
    
    /* Final blend with complex pattern */
    result = __builtin_shuffle(ab_blend, cd_blend, 
                              (v8si){0, 9, 2, 11, 4, 13, 6, 15});
    
    /* Add constant coefficients */
    result = result * (v8sf){1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    result = result + (v8sf){0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f};
    
    return result;
}

/* Complex multiply-add with many constants */
static v4sf complex_fma_pattern(v4sf a, v4sf b, v4sf c, v4sf d) {
    /* Fused multiply-add pattern with many constants */
    v4sf result;
    
    /* Complex expression that might expand to 10 operands */
    result = a * (v4sf){1.414f, 2.718f, 3.142f, 1.618f} + 
             b * (v4sf){0.577f, 1.732f, 2.236f, 2.828f} + 
             c * (v4sf){0.693f, 1.099f, 1.386f, 1.609f} + 
             d * (v4sf){0.434f, 0.868f, 1.151f, 1.442f};
    
    /* Additional shuffle to create complex pattern */
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    return result;
}

/* Atomic operation simulation with many parameters */
static long atomic_complex_op(volatile long *ptr, long a, long b, long c, long d) {
    /* Complex atomic operation pattern */
    long result = 0;
    
    /* Series of atomic operations that might be combined */
    result = __atomic_fetch_add(ptr, a, __ATOMIC_SEQ_CST);
    result = __atomic_fetch_and(ptr, b, __ATOMIC_SEQ_CST);
    result = __atomic_fetch_or(ptr, c, __ATOMIC_SEQ_CST);
    result = __atomic_fetch_xor(ptr, d, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Main test function */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;
    
    /* Initialize vectors with volatile to prevent constant propagation */
    volatile v4si v1 = {1, 2, 3, 4};
    volatile v4si v2 = {5, 6, 7, 8};
    volatile v4si v3 = {9, 10, 11, 12};
    volatile v4si v4 = {13, 14, 15, 16};
    
    volatile v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    volatile v4sf f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    volatile v4sf f3 = {9.0f, 10.0f, 11.0f, 12.0f};
    volatile v4sf f4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    volatile v8sf f8_1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    volatile v8sf f8_2 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    volatile v8sf f8_3 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f};
    volatile v8sf f8_4 = {25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    
    volatile long atomic_var = 0;
    volatile long atomic_result = 0;
    
    v4si int_result = {0, 0, 0, 0};
    v4sf float_result = {0.0f, 0.0f, 0.0f, 0.0f};
    v8sf float8_result = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Loop to prevent optimization and ensure execution */
    for (int i = 0; i < iterations; i++) {
        /* Test 1: Complex shuffle with 10 operands */
        int_result = complex_shuffle_10_operands(
            (v4si)v1 + i, 
            (v4si)v2 + i, 
            (v4si)v3 + i, 
            (v4si)v4 + i
        );
        
        /* Test 2: Complex vector blend */
        float8_result = vector_blend_complex(
            (v8sf)f8_1 + (float)i,
            (v8sf)f8_2 + (float)i,
            (v8sf)f8_3 + (float)i,
            (v8sf)f8_4 + (float)i
        );
        
        /* Test 3: Complex FMA pattern */
        float_result = complex_fma_pattern(
            (v4sf)f1 + (float)i,
            (v4sf)f2 + (float)i,
            (v4sf)f3 + (float)i,
            (v4sf)f4 + (float)i
        );
        
        /* Test 4: Atomic operations */
        atomic_result = atomic_complex_op(
            (volatile long *)&atomic_var,
            i * 10 + 1,
            i * 10 + 2,
            i * 10 + 3,
            i * 10 + 4
        );
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", 
           int_result[0], int_result[1], int_result[2], int_result[3]);
    printf("Float results: %f %f %f %f\n",
           float_result[0], float_result[1], float_result[2], float_result[3]);
    printf("Atomic result: %ld\n", atomic_result);
    
    /* Print first element of v8 result */
    printf("V8 result[0]: %f\n", float8_result[0]);
    
    return 0;
}
