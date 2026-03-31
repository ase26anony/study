/* test_optabs.c - Program to trigger 10-operand expansion in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle operation using multiple vectors and immediate controls */
static v4si complex_shuffle_10_operands(v4si a, v4si b, v4si c, v4si d) {
    /* This should trigger a 10-operand expansion:
     * 4 source vectors + 6 immediate control values = 10 operands
     */
    v4si result;
    
    /* Use __builtin_shuffle with multiple vectors and complex control */
    /* Control mask: {0, 4, 1, 5, 2, 6, 3, 7} - interleaving elements */
    result = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    
    /* Another shuffle with different control pattern */
    v4si temp = __builtin_shuffle(c, d, (v4si){2, 6, 3, 7});
    
    /* Combine results with bitwise operations using multiple constants */
    result = result & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    temp = temp | (v4si){0x1, 0x100, 0x10000, 0x1000000};
    
    /* Final operation with multiple operands */
    result = result ^ temp ^ (v4si){0x55, 0xAA, 0x55, 0xAA};
    
    return result;
}

/* Vector FMA-like operation with many constants */
static v4sf vector_polynomial(v4sf x) {
    /* Polynomial evaluation: ax^3 + bx^2 + cx + d
     * With vector coefficients, this requires many operands */
    v4sf a = {3.14159f, 2.71828f, 1.41421f, 1.61803f};
    v4sf b = {2.0f, 2.5f, 3.0f, 3.5f};
    v4sf c = {1.0f, 1.5f, 2.0f, 2.5f};
    v4sf d = {0.5f, 1.0f, 1.5f, 2.0f};
    
    /* x^2 and x^3 */
    v4sf x2 = x * x;
    v4sf x3 = x2 * x;
    
    /* Polynomial with many operands */
    return (a * x3) + (b * x2) + (c * x) + d;
}

/* Complex number multiplication that might expand to many operands */
static void complex_mul(float ar, float ai, float br, float bi,
                        float *cr, float *ci) {
    /* (a+bi)*(c+di) = (ac-bd) + (ad+bc)i
     * This might be expanded into internal function with many operands */
    *cr = ar * br - ai * bi;
    *ci = ar * bi + ai * br;
}

/* Atomic operation with many parameters (if supported) */
static long atomic_cmpxchg_10_params(volatile long *ptr, long oldval, long newval,
                                     int memorder1, int memorder2,
                                     int memorder3, int memorder4,
                                     int memorder5, int memorder6) {
    /* Complex atomic operation with many ordering parameters */
    return __atomic_compare_exchange_n(ptr, &oldval, newval, 
                                       0, memorder1, memorder2);
}

int main(int argc, char *argv[]) {
    volatile int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations > 10) iterations = 10;
    }
    
    /* Initialize vectors with different patterns */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    v4si vec4 = {13, 14, 15, 16};
    
    v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    
    v4si result_int = {0};
    v4sf result_float = {0.0f};
    
    /* Loop to prevent optimization but not unroll completely */
    for (volatile int i = 0; i < iterations; i++) {
        /* Operation 1: Complex shuffle with many operands */
        result_int = complex_shuffle_10_operands(vec1, vec2, vec3, vec4);
        
        /* Operation 2: Vector polynomial with many constants */
        result_float = vector_polynomial(fvec);
        
        /* Operation 3: Complex multiplication */
        float cr, ci;
        complex_mul(1.0f, 2.0f, 3.0f, 4.0f, &cr, &ci);
        
        /* Mix results to prevent dead code elimination */
        vec1 = vec1 + (v4si){result_int[0], 0, 0, 0};
        fvec = fvec + (v4sf){cr, ci, 0.0f, 0.0f};
    }
    
    /* Print results to create side effects */
    printf("Result int: %d %d %d %d\n", 
           result_int[0], result_int[1], result_int[2], result_int[3]);
    printf("Result float: %f %f %f %f\n",
           result_float[0], result_float[1], result_float[2], result_float[3]);
    
    /* Try to use __builtin_convertvector with complex pattern */
    v4si int_vec = {100, 200, 300, 400};
    v4sf float_vec = __builtin_convertvector(int_vec, v4sf);
    printf("Converted: %f %f %f %f\n",
           float_vec[0], float_vec[1], float_vec[2], float_vec[3]);
    
    return 0;
}
