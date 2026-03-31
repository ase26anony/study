/* Test program to cover 10-11 operand cases in optabs.cc */
/* Compile with: gcc -O3 -mavx512f -mfma -ftree-vectorize -c test.c -o test.o */

#include <immintrin.h>
#include <stdio.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* Complex AVX-512 operation with many operands */
FORCE_INLINE __m512 test_avx512_10_operand(__m512 a, __m512 b, __m512 c, 
                                          __m512 d, __m512 e, __mmask16 k) {
    /* This should expand to a pattern with many operands:
     * 1. Result register
     * 2. Mask register
     * 3-7. Input registers a-e
     * 8. Constant 1.0f
     * 9. Constant 2.0f
     * 10. FMA control
     */
    __m512 one = _mm512_set1_ps(1.0f);
    __m512 two = _mm512_set1_ps(2.0f);
    
    /* Complex expression that might use 10+ operands in RTL expansion */
    __m512 t1 = _mm512_mask_fmadd_ps(a, k, b, c);  /* 5 operands */
    __m512 t2 = _mm512_mask_fmadd_ps(d, k, e, one); /* 5 operands */
    __m512 result = _mm512_mask_add_ps(t1, k, t1, t2); /* 4 operands */
    
    /* Additional operation to ensure all values are used */
    return _mm512_mask_mul_ps(result, k, result, two);
}

/* Test with inline assembly having 11 operands */
void test_inline_asm_11_operands(void) {
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2;
    
    /* Inline asm with 11 operands: 2 outputs + 9 inputs = 11 total */
    asm volatile (
        "add %[r1], %[a1], %[a2]\n\t"
        "add %[r2], %[a3], %[a4]\n\t"
        "mul %[r1], %[r1], %[a5]\n\t"
        "add %[r2], %[r2], %[a6]\n\t"
        "sub %[r1], %[r1], %[a7]\n\t"
        "add %[r2], %[r2], %[a8]\n\t"
        "and %[r1], %[r1], %[a9]\n\t"
        "or  %[r2], %[r2], %[a10]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10)
        : "cc"
    );
    
    printf("ASM result: %ld, %ld\n", result1, result2);
}

/* Use GCC vector extensions for complex operations */
typedef float v16sf __attribute__((vector_size(64)));

FORCE_INLINE v16sf test_gcc_vector_ops(v16sf a, v16sf b, v16sf c, 
                                       v16sf d, v16sf e, v16sf f) {
    /* Complex expression that might generate many operands */
    v16sf t1 = a * b + c;
    v16sf t2 = d * e + f;
    v16sf t3 = t1 * t2 - a;
    v16sf t4 = t3 * b + c * d;
    
    /* Use FMA builtins which can expand to multi-operand patterns */
    v16sf result = __builtin_fma(t1, t2, t3);
    result = __builtin_fma(result, t4, a);
    
    return result + b * c - d;
}

/* OpenMP SIMD reduction with vector types */
void test_omp_reduction(void) {
    v16sf array[100];
    v16sf sum = {0};
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        array[i] = (v16sf){i * 0.1f};
    }
    
    /* Complex reduction that might generate multi-operand patterns */
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        v16sf temp = array[i];
        temp = temp * temp + array[i] * 2.0f;
        sum = sum + temp;
    }
    
    /* Use the result */
    float* p = (float*)&sum;
    printf("Reduction sum[0]: %f\n", p[0]);
}

/* Main test function */
int main(void) {
    /* Initialize AVX-512 vectors */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __m512 d = _mm512_set1_ps(4.0f);
    __m512 e = _mm512_set1_ps(5.0f);
    __mmask16 mask = 0xAAAA;  /* Alternating bits */
    
    /* Test AVX-512 operations */
    __m512 result = test_avx512_10_operand(a, b, c, d, e, mask);
    
    /* Extract and print a value to prevent optimization */
    float res_array[16];
    _mm512_storeu_ps(res_array, result);
    printf("AVX-512 result[0]: %f\n", res_array[0]);
    
    /* Test inline assembly */
    test_inline_asm_11_operands();
    
    /* Test GCC vector extensions */
    v16sf v1 = {0}, v2 = {1}, v3 = {2}, v4 = {3}, v5 = {4}, v6 = {5};
    v16sf vec_result = test_gcc_vector_ops(v1, v2, v3, v4, v5, v6);
    
    /* Test OpenMP reduction */
    test_omp_reduction();
    
    return 0;
}
