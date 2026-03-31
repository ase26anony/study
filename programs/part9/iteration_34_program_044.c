/* test_optabs.c - Program to trigger 10-operand expansion in GCC optabs */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Complex shuffle with many operands - designed to require 10 operands during expansion */
static v4si complex_vector_shuffle(v4si a, v4si b, v4si c, v4si d) {
    /* This complex expression should expand to something needing many operands:
     * 1. Multiple source vectors (a, b, c, d)
     * 2. Multiple immediate control values for shuffling
     * 3. Multiple arithmetic operations with constants
     */
    
    /* Create a complex shuffle pattern using multiple builtins and constants */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Mix with arithmetic operations using many constants */
    v4si result = temp1 * (v4si){3, 7, 11, 13} + 
                  temp2 * (v4si){2, 5, 17, 19} +
                  (a & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000}) |
                  (b & (v4si){0xF0F0F0F0, 0x0F0F0F0F, 0xCCCCCCCC, 0x33333333});
    
    /* Additional complex shuffle with immediate control */
    result = __builtin_shuffle(result, temp1 + temp2, 
                              (v4si){3, 0, 1, 2}) ^ 
             (v4si){0xAAAAAAAA, 0x55555555, 0xAAAAAAAA, 0x55555555};
    
    return result;
}

/* Vector FMA-like operation with many constants */
static v4sf vector_polynomial(v4sf x, v4sf coeff0, v4sf coeff1, 
                             v4sf coeff2, v4sf coeff3) {
    /* Polynomial evaluation: coeff0 + x*(coeff1 + x*(coeff2 + x*coeff3))
     * This creates complex expression trees that may need many operands */
    
    v4sf x2 = x * x;
    v4sf x3 = x2 * x;
    
    /* Mix with shuffle operations */
    v4sf shuffled_coeffs = __builtin_shuffle(coeff0, coeff1, 
                                            (v4si){0, 5, 2, 7});
    v4sf shuffled_more = __builtin_shuffle(coeff2, coeff3, 
                                          (v4si){4, 1, 6, 3});
    
    /* Complex expression with many operands */
    return (shuffled_coeffs * (v4sf){1.0f, 2.0f, 3.0f, 4.0f}) +
           (shuffled_more * (v4sf){5.0f, 6.0f, 7.0f, 8.0f}) +
           (x * (v4sf){0.1f, 0.2f, 0.3f, 0.4f}) +
           (x2 * (v4sf){0.01f, 0.02f, 0.03f, 0.04f}) +
           (x3 * (v4sf){0.001f, 0.002f, 0.003f, 0.004f});
}

/* Atomic-style operation simulation with many parameters */
static v2di complex_atomic_op(v2di *ptr, v2di a, v2di b, v2di c, v2di d) {
    /* Simulate complex atomic operation requiring many operands */
    v2di old = *ptr;
    
    /* Complex expression mixing multiple vectors and constants */
    v2di result = (old + a) * (v2di){3LL, 7LL} -
                  (b - c) * (v2di){2LL, 5LL} +
                  (d & (v2di){0xFFFFFFFFLL, 0xFFFFFFFF00000000LL}) |
                  (old ^ (v2di){0xAAAAAAAAAAAAAAAALL, 0x5555555555555555LL});
    
    /* Shuffle with immediate control */
    result = __builtin_shuffle(result, a + b + c + d, 
                              (v2di){1, 0});
    
    return result;
}

/* Main test function */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;
    
    /* Initialize vectors with pattern data */
    v4si vec_a = (v4si){1, 2, 3, 4};
    v4si vec_b = (v4si){5, 6, 7, 8};
    v4si vec_c = (v4si){9, 10, 11, 12};
    v4si vec_d = (v4si){13, 14, 15, 16};
    
    v4sf fvec_a = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf coeff0 = (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    v4sf coeff1 = (v4sf){0.25f, 0.75f, 1.25f, 1.75f};
    v4sf coeff2 = (v4sf){0.125f, 0.375f, 0.625f, 0.875f};
    v4sf coeff3 = (v4sf){0.0625f, 0.1875f, 0.3125f, 0.4375f};
    
    v2di atomic_data = (v2di){0x123456789ABCDEF0LL, 0xFEDCBA9876543210LL};
    v2di atomic_a = (v2di){1LL, 2LL};
    v2di atomic_b = (v2di){3LL, 4LL};
    v2di atomic_c = (v2di){5LL, 6LL};
    v2di atomic_d = (v2di){7LL, 8LL};
    
    v4si final_result_int = (v4si){0, 0, 0, 0};
    v4sf final_result_float = (v4sf){0.0f, 0.0f, 0.0f, 0.0f};
    v2di final_result_long = (v2di){0LL, 0LL};
    
    /* Loop to prevent optimization but keep it small */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call complex operations that should trigger 10-operand expansions */
        final_result_int = complex_vector_shuffle(vec_a + i, vec_b + i, 
                                                 vec_c + i, vec_d + i);
        
        final_result_float = vector_polynomial(fvec_a + (float)i, 
                                              coeff0, coeff1, coeff2, coeff3);
        
        final_result_long = complex_atomic_op(&atomic_data, 
                                             atomic_a + i, atomic_b + i, 
                                             atomic_c + i, atomic_d + i);
        
        /* Mix results to create data dependencies */
        vec_a = vec_a + final_result_int;
        fvec_a = fvec_a + final_result_float;
        atomic_data = atomic_data + final_result_long;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d %d %d %d\n", 
           final_result_int[0], final_result_int[1], 
           final_result_int[2], final_result_int[3]);
    printf("Float results: %f %f %f %f\n",
           final_result_float[0], final_result_float[1],
           final_result_float[2], final_result_float[3]);
    printf("Long results: %lld %lld\n",
           final_result_long[0], final_result_long[1]);
    
    return 0;
}
