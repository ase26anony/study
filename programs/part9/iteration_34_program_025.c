/* test_optabs.c - Target GCC's 10-operand expansion path */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));

/* Complex shuffle operation requiring many operands */
static v4si complex_vector_shuffle_10op(v4si a, v4si b, v4si c, v4si d) {
    /* This complex shuffle pattern conceptually needs:
     * 4 source vectors (a, b, c, d) = 4 operands
     * 6 control constants for shuffle indices = 6 operands
     * Total: 10 operands
     */
    
    /* Create a complex shuffle using multiple builtins and conversions */
    v4si temp1 = __builtin_shuffle(a, b, (v4si){0, 5, 2, 7});
    v4si temp2 = __builtin_shuffle(c, d, (v4si){4, 1, 6, 3});
    
    /* Mix with immediate constants in complex pattern */
    v4si result = temp1 + (v4si){1, 2, 3, 4};
    result = result ^ temp2;
    result = result & (v4si){0xFF, 0xFF00, 0xFF0000, 0xFF000000};
    
    /* Additional complex operation with many constants */
    result = (result << (v4si){1, 2, 3, 4}) | 
             (result >> (v4si){31, 30, 29, 28});
    
    return result;
}

/* Vector FMA-like operation with many constants */
static v8sf vector_polynomial_10op(v8sf x, v8sf coeff0, v8sf coeff1) {
    /* Polynomial evaluation: coeff0 + x*coeff1 + x*x*2.5 + x*x*x*1.5 + ... */
    v8sf x2 = x * x;
    v8sf x3 = x2 * x;
    v8sf x4 = x3 * x;
    
    /* Complex expression with many constants - may expand to 10 operands */
    v8sf result = coeff0 + 
                  x * coeff1 + 
                  x2 * (v8sf){2.5f, 2.5f, 2.5f, 2.5f, 2.5f, 2.5f, 2.5f, 2.5f} +
                  x3 * (v8sf){1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f} +
                  x4 * (v8sf){0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    
    /* Additional mixing */
    result = __builtin_shuffle(result, result, 
              (v8si){0, 7, 2, 5, 4, 3, 6, 1});
    
    return result;
}

/* Atomic-style complex operation */
static long atomic_multi_op(long *ptr, int a, int b, int c, int d, int e, int f) {
    /* Complex atomic operation pattern */
    long old = __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
    long new_val = old + a * b + c * d + e * f;
    
    /* Try to exchange with multiple conditions */
    if (__atomic_compare_exchange_n(ptr, &old, new_val, 
                                    0, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)) {
        return old;
    }
    
    /* Complex fallback with many operations */
    return old + (a << b) + (c >> d) + (e & f);
}

/* Main test function */
int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations < 1) iterations = 1;
    
    /* Initialize vector data */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_d = {13, 14, 15, 16};
    
    v8sf vec_x = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf coeff0 = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    v8sf coeff1 = {1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f};
    
    long atomic_var = 0;
    
    /* Loop to prevent optimization */
    v4si int_result = {0, 0, 0, 0};
    v8sf float_result = {0};
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Call 10-operand vector shuffle */
        int_result = complex_vector_shuffle_10op(vec_a, vec_b, vec_c, vec_d);
        
        /* Call 10-operand polynomial evaluation */
        float_result = vector_polynomial_10op(vec_x, coeff0, coeff1);
        
        /* Complex atomic operation */
        long atomic_res = atomic_multi_op(&atomic_var, 
                                          i, i+1, i+2, i+3, i+4, i+5);
        atomic_var += atomic_res;
        
        /* Modify inputs slightly */
        vec_a += (v4si){1, 1, 1, 1};
        vec_x += (v8sf){0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
    }
    
    /* Use results to prevent dead code elimination */
    int sum_int = int_result[0] + int_result[1] + 
                  int_result[2] + int_result[3];
    
    float sum_float = float_result[0] + float_result[1] + 
                      float_result[2] + float_result[3] +
                      float_result[4] + float_result[5] + 
                      float_result[6] + float_result[7];
    
    printf("Results: int_sum=%d, float_sum=%.2f, atomic=%ld\n",
           sum_int, sum_float, atomic_var);
    
    return (sum_int > 0 && sum_float > 0) ? 0 : 1;
}

/* Additional complex builtin usage that might trigger 10-operand expansion */
v8si test_multi_operand_builtin(v8si a, v8si b, v8si c, v8si d) {
    /* Complex pattern that might use table lookup or cryptographic operation */
    v8si result = a;
    
    /* Multiple operations with immediate constants */
    result = __builtin_ia32_paddd128(result, b);
    result = __builtin_ia32_pslldi128(result, 3);
    result = __builtin_ia32_pxor128(result, c);
    result = __builtin_ia32_pand128(result, d);
    
    /* Complex shuffle with many control bits */
    result = __builtin_shuffle(result, result, 
              (v8si){0, 7, 1, 6, 2, 5, 3, 4});
    
    /* Additional mixing */
    result = result | (v8si){0x00000001, 0x00000002, 0x00000004, 0x00000008,
                             0x00000010, 0x00000020, 0x00000040, 0x00000080};
    
    return result;
}
