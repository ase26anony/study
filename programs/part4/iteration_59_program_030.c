/* Test program to cover 10 and 11-operand cases in optabs.cc */
/* Compile with: gcc -O3 -mavx512f -mfma -ftree-vectorize -fno-math-errno -ffast-math -c test.c -o test.o */

#include <immintrin.h>
#include <stdio.h>

/* GCC vector extensions for complex operations */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

/* Always inline to force expansion */
__attribute__((always_inline, hot))
static inline __m512 test_avx512_10_operand(__m512 a, __m512 b, __m512 c, 
                                           __m512 d, __mmask16 k) {
    /* AVX-512 masked FMA with multiple operands - may expand to 10+ operands */
    /* _mm512_mask_fmadd_ps has 5 parameters but expands to more RTL operands */
    __m512 result = _mm512_mask_fmadd_ps(a, k, b, c);
    
    /* Additional masked operation to increase operand count in expansion */
    result = _mm512_mask_add_ps(result, k, result, d);
    
    /* Complex blend operation */
    result = _mm512_mask_blend_ps(k, a, result);
    
    return result;
}

__attribute__((always_inline, hot))
static inline __m512 test_avx512_11_operand(__m512 a, __m512 b, __m512 c,
                                           __m512 d, __m512 e, __mmask16 k1,
                                           __mmask16 k2) {
    /* Nested masked operations that may require many operands during expansion */
    __m512 t1 = _mm512_maskz_fmadd_ps(k1, a, b, c);
    __m512 t2 = _mm512_mask3_fmadd_ps(d, e, t1, k2);
    
    /* Complex permute with mask */
    __m512 result = _mm512_mask_permute_ps(t2, k1, t1, _MM_PERM_ABCD);
    
    /* Additional operation with immediate constant */
    result = _mm512_mask_add_round_ps(result, k2, result, a, _MM_FROUND_NO_EXC);
    
    return result;
}

/* Test with GCC vector extensions */
__attribute__((hot))
v16sf test_gcc_vector_ops(v16sf a, v16sf b, v16sf c, v16sf d) {
    /* Complex expression that may generate multi-operand patterns */
    v16sf result = a * b + c * d;
    
    /* Nested FMA-like operations */
    result = result + a * c + b * d;
    
    /* Conditional operations */
    v16si mask = (v16si)(a > b);
    result = (v16sf)(((v16si)result & mask) | ((v16si)c & ~mask));
    
    return result;
}

/* Inline assembly with many operands */
__attribute__((hot))
void test_many_operand_asm(void) {
    /* 11-operand inline assembly statement */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2;
    
    asm volatile (
        "mov %[r1], %[o1]\n\t"
        "add %[r1], %[o2]\n\t"
        "add %[r1], %[o3]\n\t"
        "mov %[r2], %[o4]\n\t"
        "add %[r2], %[o5]\n\t"
        "add %[r2], %[o6]\n\t"
        "imul %[r1], %[o7]\n\t"
        "imul %[r2], %[o8]\n\t"
        "add %[r1], %[o9]\n\t"
        "add %[r2], %[o10]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [o1] "r" (op1), [o2] "r" (op2), [o3] "r" (op3),
          [o4] "r" (op4), [o5] "r" (op5), [o6] "r" (op6),
          [o7] "r" (op7), [o8] "r" (op8), [o9] "r" (op9),
          [o10] "r" (op10)
        : "cc"
    );
    
    printf("ASM result: %ld, %ld\n", result1, result2);
}

/* OpenMP SIMD reduction with vector types */
__attribute__((hot))
float test_omp_reduction(void) {
    #define N 1024
    float array[N];
    float result = 0.0f;
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        array[i] = (float)i;
    }
    
    /* Complex reduction that may generate multi-operand patterns */
    #pragma omp simd reduction(+:result) simdlen(16)
    for (int i = 0; i < N; i++) {
        /* Complex expression to encourage multi-operand expansion */
        result += array[i] * 2.0f + array[i] * array[i] / 3.0f;
    }
    
    return result;
}

/* Built-in FMA operations */
__attribute__((hot))
double test_builtin_fma(double a, double b, double c, double d) {
    /* Nested FMA calls that may expand to multi-operand patterns */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, a, b);
    double t3 = __builtin_fma(t1, t2, c);
    double result = __builtin_fma(t3, a, __builtin_fma(b, c, d));
    
    return result;
}

int main(void) {
    /* Initialize test data */
    __m512 avx_vec1 = _mm512_set1_ps(1.0f);
    __m512 avx_vec2 = _mm512_set1_ps(2.0f);
    __m512 avx_vec3 = _mm512_set1_ps(3.0f);
    __m512 avx_vec4 = _mm512_set1_ps(4.0f);
    __m512 avx_vec5 = _mm512_set1_ps(5.0f);
    __mmask16 mask1 = 0xAAAA;
    __mmask16 mask2 = 0x5555;
    
    /* Test 10-operand pattern */
    __m512 result1 = test_avx512_10_operand(avx_vec1, avx_vec2, avx_vec3, avx_vec4, mask1);
    
    /* Test 11-operand pattern */
    __m512 result2 = test_avx512_11_operand(avx_vec1, avx_vec2, avx_vec3, avx_vec4, avx_vec5, mask1, mask2);
    
    /* Test GCC vector extensions */
    v16sf gcc_vec1 = {0};
    v16sf gcc_vec2 = {0};
    v16sf gcc_vec3 = {0};
    v16sf gcc_vec4 = {0};
    v16sf gcc_result = test_gcc_vector_ops(gcc_vec1, gcc_vec2, gcc_vec3, gcc_vec4);
    
    /* Test inline assembly with many operands */
    test_many_operand_asm();
    
    /* Test OpenMP reduction */
    float omp_result = test_omp_reduction();
    
    /* Test built-in FMA */
    double fma_result = test_builtin_fma(1.0, 2.0, 3.0, 4.0);
    
    /* Use results to prevent dead code elimination */
    float sum = _mm512_reduce_add_ps(result1) + 
                _mm512_reduce_add_ps(result2) +
                gcc_result[0] + omp_result + (float)fma_result;
    
    printf("Final result: %f\n", sum);
    
    return (int)sum;
}
