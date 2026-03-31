/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Use target-specific vector intrinsics */
#ifdef __AVX512F__
#include <immintrin.h>
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Strategy 2: GCC vector extensions */
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));

/* Hot function attribute to encourage complex instruction patterns */
__attribute__((hot, noinline))
void test_vector_intrinsics() {
#ifdef __AVX512F__
    /* AVX-512 mask operations can generate many operands */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __mmask16 mask = 0xAAAA;
    
    /* Complex FMA chain - each intrinsic expands to multiple operands */
    __m512 result = _mm512_mask_fmadd_ps(a, mask, b, c);
    result = _mm512_mask_add_ps(result, mask, result, a);
    result = _mm512_mask_mul_ps(result, mask, result, b);
    
    /* Use result to prevent optimization */
    volatile __m512 sink = result;
#endif

#ifdef __ARM_NEON
    /* ARM NEON multi-vector loads can generate many operands */
    int8x16x4_t vec4;
    int8x16_t src = vdupq_n_s8(1);
    
    /* Complex permute/table lookup patterns */
    vec4.val[0] = src;
    vec4.val[1] = vaddq_s8(src, vdupq_n_s8(1));
    vec4.val[2] = vaddq_s8(src, vdupq_n_s8(2));
    vec4.val[3] = vaddq_s8(src, vdupq_n_s8(3));
    
    volatile int8x16x4_t sink = vec4;
#endif
}

/* Strategy 3: Complex inline assembly with many operands */
__attribute__((noinline))
int test_multi_operand_asm() {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int k = 11;
    int result;
    
    /* Inline assembly with 11 operands (10 inputs + 1 output) */
    asm volatile (
        "add %[a], %[b], %[c]\n\t"
        "add %[d], %[e], %[f]\n\t"
        "add %[g], %[h], %[i]\n\t"
        "mul %[result], %[j], %[k]\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

/* Strategy 4: Built-in functions and complex math */
__attribute__((hot, noinline))
double test_complex_builtins() {
    double a = 1.1, b = 2.2, c = 3.3, d = 4.4, e = 5.5;
    double f = 6.6, g = 7.7, h = 8.8, i = 9.9, j = 10.10;
    
    /* Chain of FMA operations - each expands to multiple operands */
    double result = __builtin_fma(a, b, c);
    result = __builtin_fma(result, d, e);
    result = __builtin_fma(result, f, g);
    result = __builtin_fma(result, h, i);
    result = __builtin_fma(result, j, a);
    
    /* Complex expression that might generate many operands */
    result = result * __builtin_fma(b, c, d) / __builtin_fma(e, f, g);
    
    return result;
}

/* Strategy 5: GCC vector extensions with complex operations */
__attribute__((hot, noinline))
v8sf test_vector_extensions() {
    v8sf v1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    v8sf v3 = {3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
    v8sf v4 = {4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    
    /* Complex vector expression that might generate multi-operand patterns */
    v8sf result = v1 * v2 + v3 / v4;
    result = result + v1 * v3 - v2 * v4;
    result = __builtin_fmaf(result, v1, v2);
    
    return result;
}

/* Strategy: OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
__attribute__((hot, noinline))
float test_omp_reduction() {
    #define N 1024
    float array[N];
    for (int i = 0; i < N; i++) {
        array[i] = (float)i;
    }
    
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += array[i] * array[i];
    }
    
    return sum;
}
#endif

/* Main function that exercises all strategies */
int main() {
    printf("Testing multi-operand instruction patterns...\n");
    
    /* Test 1: Vector intrinsics */
    test_vector_intrinsics();
    
    /* Test 2: Multi-operand inline assembly */
    int asm_result = test_multi_operand_asm();
    printf("Assembly test result: %d\n", asm_result);
    
    /* Test 3: Complex builtins */
    double builtin_result = test_complex_builtins();
    printf("Builtin test result: %f\n", builtin_result);
    
    /* Test 4: Vector extensions */
    v8sf vec_result = test_vector_extensions();
    printf("Vector test result: %f (first element)\n", vec_result[0]);
    
    #ifdef _OPENMP
    /* Test 5: OpenMP reduction */
    float omp_result = test_omp_reduction();
    printf("OpenMP reduction result: %f\n", omp_result);
    #endif
    
    return 0;
}
