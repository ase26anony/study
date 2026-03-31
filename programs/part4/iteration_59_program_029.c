/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Strategy 1: Use AVX-512 intrinsics with many operands (for x86 targets) */
#ifdef __AVX512F__
#include <immintrin.h>

/* Force inline expansion */
static __m512 __attribute__((always_inline))
test_avx512_10_operands(__m512 a, __m512 b, __m512 c, __m512 d,
                       __mmask16 k, __m512 src, float imm) {
    /* _mm512_mask3_fmadd_ps has implicit 3 sources + mask + rounding mode = many operands */
    /* Combine multiple masked operations to increase operand count */
    __m512 t1 = _mm512_maskz_fmadd_ps(k, a, b, c);  /* mask, 3 sources */
    __m512 t2 = _mm512_mask_fmadd_ps(src, k, d, t1); /* src, mask, 2 sources */
    
    /* Use broadcast with mask - adds more operands */
    __m512 t3 = _mm512_mask_broadcast_f32x4(t2, k, _mm_set1_ps(imm));
    
    /* Complex permute with many operands */
    return _mm512_mask_permute4f128_ps(t3, k, t2, _MM_PERM_BADC);
}

/* Test case with potential 11 operands */
static __m512i __attribute__((always_inline))
test_avx512_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                       __mmask64 k, int imm1, int imm2, int imm3) {
    /* Multiple immediate operands plus vector sources */
    __m512i t1 = _mm512_mask_slli_epi32(a, k, b, imm1);
    __m512i t2 = _mm512_mask_srai_epi32(t1, k, c, imm2);
    return _mm512_mask_alignr_epi32(d, k, t2, b, imm3);
}
#endif

/* Strategy 2: Use GCC vector extensions for complex operations */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

/* Complex expression that might generate multi-operand patterns */
static v16sf __attribute__((always_inline, hot))
test_vector_extension(v16sf a, v16sf b, v16sf c, v16sf d,
                     v16sf e, v16sf f, v16sf g) {
    /* Complex FMA-like chain - GCC might combine into multi-operand pattern */
    v16sf t1 = a * b + c;
    v16sf t2 = d * e + f;
    v16sf t3 = t1 * t2 + g;
    v16sf t4 = t3 * a + b;
    v16sf t5 = t4 * c + d;
    
    /* Mix with broadcasts */
    v16sf broadcast = (v16sf){1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f,
                              1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f, 1.5f};
    
    return t5 * broadcast + e;
}

/* Strategy 3: Inline assembly with many operands */
static void __attribute__((always_inline))
test_multi_operand_asm(void) {
    /* Create 11 distinct variables to use as operands */
    uint64_t out1, out2, out3;
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    uint64_t in6 = 6, in7 = 7;
    
    /* Extended asm with 10 operands (2 outputs, 7 inputs, 1 clobber) */
    asm volatile (
        "mov %[o1], %[i1]\n\t"
        "add %[o1], %[i2]\n\t"
        "mov %[o2], %[i3]\n\t"
        "imul %[o2], %[i4]\n\t"
        "mov %[o3], %[i5]\n\t"
        "lea (%[o3], %[i6], 2), %[o3]\n\t"
        : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6),
          [i7] "r" (in7)
        : "cc", "memory"
    );
    
    /* Use results to prevent optimization */
    printf("ASM results: %lu %lu %lu\n", out1, out2, out3);
}

/* Strategy 4: Complex reduction with OpenMP SIMD */
#ifdef _OPENMP
static float test_omp_reduction(float* arr, int n) {
    float sum = 0.0f;
    
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        sum += arr[i] * arr[i] + 1.0f;
    }
    
    return sum;
}
#endif

/* Strategy 5: Built-in math functions that expand to multi-operand patterns */
static double __attribute__((always_inline))
test_builtin_fma_chain(double a, double b, double c, double d,
                      double e, double f, double g) {
    /* Chain of FMA operations - might expand to multi-operand pattern */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(t1, t2, g);
    double t4 = __builtin_fma(t3, a, b);
    return __builtin_fma(t4, c, d);
}

/* Main test driver */
int main() {
    float result = 0.0f;
    
    /* Test 1: Vector extensions */
    {
        v16sf a = {1.0f}, b = {2.0f}, c = {3.0f}, d = {4.0f};
        v16sf e = {5.0f}, f = {6.0f}, g = {7.0f};
        v16sf res = test_vector_extension(a, b, c, d, e, f, g);
        result += res[0];
    }
    
    /* Test 2: Inline assembly */
    test_multi_operand_asm();
    
    /* Test 3: Built-in FMA chain */
    {
        double r = test_builtin_fma_chain(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7);
        result += (float)r;
    }
    
#ifdef __AVX512F__
    /* Test 4: AVX-512 intrinsics */
    {
        __m512 va = _mm512_set1_ps(1.0f);
        __m512 vb = _mm512_set1_ps(2.0f);
        __m512 vc = _mm512_set1_ps(3.0f);
        __m512 vd = _mm512_set1_ps(4.0f);
        __mmask16 mask = 0xAAAA;
        
        __m512 vres = test_avx512_10_operands(va, vb, vc, vd, mask, va, 5.0f);
        float fres[16];
        _mm512_storeu_ps(fres, vres);
        result += fres[0];
    }
#endif
    
#ifdef _OPENMP
    /* Test 5: OpenMP reduction */
    {
        float arr[100];
        for (int i = 0; i < 100; i++) arr[i] = (float)i;
        result += test_omp_reduction(arr, 100);
    }
#endif
    
    printf("Final result: %f\n", result);
    return (int)result;
}
