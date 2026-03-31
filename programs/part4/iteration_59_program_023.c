/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Strategy 1: Use AVX-512 intrinsics with many operands (for x86) */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inline expansion */
static __m512 __attribute__((always_inline))
test_avx512_multi_operand(__m512 a, __m512 b, __m512 c, __m512 d,
                         __mmask16 k, float constant) {
    /* Complex expression that might expand to multiple instructions
       with many operands during RTL expansion */
    __m512 t1 = _mm512_maskz_fmadd_ps(k, a, b, c);
    __m512 t2 = _mm512_mask_sub_ps(k, d, t1, _mm512_set1_ps(constant));
    __m512 t3 = _mm512_mask_mul_ps(k, t2, a, _mm512_set1_ps(2.0f));
    
    /* Nested FMA operations - each FMA has 4 operands (mask counts as one) */
    __m512 result = _mm512_mask_fmadd_ps(k, t3, b, 
        _mm512_mask_fnmadd_ps(k, c, d, _mm512_set1_ps(3.0f)));
    
    return result;
}
#endif

/* Strategy 2: Use GCC vector extensions for complex operations */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

/* Hot function to encourage complex instruction patterns */
__attribute__((hot, noinline))
static v16sf test_vector_extension(v16sf a, v16sf b, v16sf c, 
                                   v16sf d, v16sf e, float scalar) {
    /* Complex expression with many vector operands */
    v16sf broadcast = (v16sf){scalar, scalar, scalar, scalar, 
                              scalar, scalar, scalar, scalar,
                              scalar, scalar, scalar, scalar,
                              scalar, scalar, scalar, scalar};
    
    /* Multiple operations in one expression - may generate multi-operand pattern */
    v16sf result = a * b + c * d - e * broadcast;
    result = result + a * c - b * d + e * broadcast * 2.0f;
    
    /* Conditional operation using mask */
    v16si mask = (v16si)(a > b);
    v16sf blended = __builtin_shuffle(result, a, (v16si)mask);
    
    return blended + c * 3.0f;
}

/* Strategy 3: Complex reduction pattern */
#ifdef _OPENMP
#pragma omp declare simd
#endif
static float complex_reduction(float *a, float *b, int n) {
    float sum_a = 0.0f, sum_b = 0.0f;
    
    #ifdef _OPENMP
    #pragma omp simd reduction(+:sum_a, sum_b) simdlen(16)
    #endif
    for (int i = 0; i < n; i++) {
        /* Complex expression that might expand to multi-operand FMA */
        float t1 = a[i] * b[i] + a[i] * 2.0f;
        float t2 = b[i] * a[i] - b[i] * 3.0f;
        sum_a += t1 * t2 + a[i];
        sum_b += t1 - t2 * b[i];
    }
    
    return sum_a * sum_b;
}

/* Strategy 4: Inline assembly with many operands */
static void test_multi_operand_asm(void) {
    /* Create 11 distinct variables to use as operands */
    uint64_t out1, out2, out3;
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    uint64_t in6 = 6, in7 = 7, in8 = 8;
    
    /* 11-operand asm statement (3 outputs, 8 inputs) */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "mov %1, %5\n\t"
        "add %1, %6\n\t"
        "mov %2, %7\n\t"
        "add %2, %8\n\t"
        : "=&r"(out1), "=&r"(out2), "=&r"(out3)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), 
          "r"(in5), "r"(in6), "r"(in7), "r"(in8)
        : "cc"
    );
    
    /* Use results to prevent optimization */
    printf("ASM results: %lu %lu %lu\n", out1, out2, out3);
}

/* Strategy 5: Built-in math functions with many operands */
static double test_builtin_math(double a, double b, double c, 
                                double d, double e, double f) {
    /* Nested FMA operations - each has 3 operands, but combined
       they create complex expressions during RTL expansion */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(t1, t2, a);
    double t4 = __builtin_fma(b, c, d);
    double t5 = __builtin_fma(e, f, t3);
    
    return __builtin_fma(t4, t5, __builtin_fma(a, b, c * d));
}

/* Main test driver */
int main(void) {
    int n = 1024;
    float *a = malloc(n * sizeof(float));
    float *b = malloc(n * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        a[i] = (float)i / n;
        b[i] = (float)(n - i) / n;
    }
    
    /* Test 1: Vector extensions */
    v16sf va = {0}, vb = {0}, vc = {0}, vd = {0}, ve = {0};
    for (int i = 0; i < 16; i++) {
        va[i] = i * 0.1f;
        vb[i] = i * 0.2f;
        vc[i] = i * 0.3f;
        vd[i] = i * 0.4f;
        ve[i] = i * 0.5f;
    }
    
    v16sf vresult = test_vector_extension(va, vb, vc, vd, ve, 2.5f);
    
    /* Test 2: Complex reduction */
    float reduction_result = complex_reduction(a, b, n);
    
    /* Test 3: Built-in math */
    double math_result = test_builtin_math(1.1, 2.2, 3.3, 4.4, 5.5, 6.6);
    
    /* Test 4: Multi-operand assembly */
    test_multi_operand_asm();
    
    /* Use results to prevent dead code elimination */
    printf("Vector result elem0: %f\n", (double)vresult[0]);
    printf("Reduction result: %f\n", reduction_result);
    printf("Math result: %f\n", math_result);
    
    free(a);
    free(b);
    
    return 0;
}
