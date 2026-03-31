/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* GCC vector extensions for fallback */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

/* Complex inline assembly with many operands */
static void inline_asm_11_operands(void) {
    int64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    
    /* 11 operands: 1 output + 10 inputs/clobbers */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "mov %[k], %[a]"
        : [a] "+r" (a), [c] "+r" (c), [e] "+r" (e), [g] "+r" (g), [i] "+r" (i)
        : [b] "r" (b), [d] "r" (d), [f] "r" (f), [h] "r" (h), [j] "r" (j), [k] "r" (k)
        : "cc", "memory"
    );
    
    printf("ASM result: %ld\n", a);
}

#ifdef __AVX512F__
/* AVX-512 intrinsics that can generate many operands */
FORCE_INLINE __m512 test_avx512_10_operands(__m512 a, __m512 b, __m512 c, 
                                           __m512 d, __m512 e, __m512 f,
                                           __mmask16 k1, __mmask16 k2) {
    /* Complex sequence that may expand to multi-operand patterns */
    __m512 t1 = _mm512_mask_add_ps(a, k1, b, c);  /* 5 operands */
    __m512 t2 = _mm512_mask_mul_ps(d, k2, e, f);  /* 5 operands */
    
    /* FMA with mask - up to 6 operands in intrinsic, more in RTL */
    __m512 t3 = _mm512_mask_fmadd_ps(t1, k1, t2, a);
    
    /* Blend with mask - 4 operands */
    __m512 result = _mm512_mask_blend_ps(k2, t3, b);
    
    /* Permute with immediate - 3 operands but complex expansion */
    result = _mm512_permutexvar_ps(_mm512_set1_epi32(0x0F), result);
    
    return result;
}

FORCE_INLINE __m512i test_avx512_11_operands(__m512i a, __m512i b, __m512i c,
                                            __m512i d, __m512i e, __m512i f,
                                            __mmask64 k1, int imm8) {
    /* Gather instruction with many parameters */
    __m512i indices = _mm512_add_epi32(a, b);
    
    /* Complex operation that may expand to many operands */
    __m512i t1 = _mm512_mask_slli_epi32(a, k1, b, 3);
    __m512i t2 = _mm512_mask_add_epi32(c, k1, d, e);
    
    /* Multiple operations in expression */
    __m512i result = _mm512_mask_alignr_epi32(f, k1, t1, t2, imm8);
    
    /* Another complex operation */
    result = _mm512_maskz_compress_epi32(k1, result);
    
    return result;
}
#endif

/* GCC vector extensions for complex reductions */
static v16sf vector_reduction_10_operands(v16sf a, v16sf b, v16sf c, 
                                         v16sf d, v16sf e, v16sf f) {
    /* Complex expression that may generate many operands */
    v16sf t1 = a + b * c;
    v16sf t2 = d - e / f;
    v16sf t3 = t1 * t2 + a;
    v16sf t4 = b * c - d;
    v16sf t5 = e + f * a;
    
    /* Final combination - compiler may try to combine into complex pattern */
    v16sf result = t3 * t4 + t5 * a - b + c * d - e + f;
    
    return result;
}

/* Built-in functions for complex math */
static double complex_math_10_operands(double a, double b, double c, 
                                      double d, double e, double f,
                                      double g, double h, double i, double j) {
    /* Nested FMA calls - each expands to 4 operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(g, h, i);
    
    /* Complex expression preventing early folding */
    double result = __builtin_fma(t1, t2, t3) * j;
    result = __builtin_fma(result, a, b);
    result = __builtin_fma(result, c, d);
    
    return result;
}

/* Main test function */
int main(void) {
    printf("Testing multi-operand instruction patterns...\n");
    
    /* Test inline assembly path */
    inline_asm_11_operands();
    
#ifdef __AVX512F__
    /* Initialize AVX-512 vectors */
    __m512 avx1 = _mm512_set1_ps(1.0f);
    __m512 avx2 = _mm512_set1_ps(2.0f);
    __m512 avx3 = _mm512_set1_ps(3.0f);
    __m512 avx4 = _mm512_set1_ps(4.0f);
    __m512 avx5 = _mm512_set1_ps(5.0f);
    __m512 avx6 = _mm512_set1_ps(6.0f);
    
    __m512i avxi1 = _mm512_set1_epi32(1);
    __m512i avxi2 = _mm512_set1_epi32(2);
    __m512i avxi3 = _mm512_set1_epi32(3);
    __m512i avxi4 = _mm512_set1_epi32(4);
    __m512i avxi5 = _mm512_set1_epi32(5);
    __m512i avxi6 = _mm512_set1_epi32(6);
    
    __mmask16 k1 = 0xAAAA;
    __mmask16 k2 = 0x5555;
    __mmask64 k64 = 0xAAAAAAAAAAAAAAAA;
    
    /* Test AVX-512 paths */
    __m512 r1 = test_avx512_10_operands(avx1, avx2, avx3, avx4, avx5, avx6, k1, k2);
    __m512i r2 = test_avx512_11_operands(avxi1, avxi2, avxi3, avxi4, avxi5, avxi6, k64, 4);
    
    /* Force use of results */
    float fr = _mm512_cvtss_f32(r1);
    int ir = _mm512_cvtsi512_si32(r2);
    printf("AVX-512 results: %f, %d\n", fr, ir);
#endif
    
    /* Test GCC vector extensions */
    v16sf v1 = {1.0f}, v2 = {2.0f}, v3 = {3.0f}, v4 = {4.0f}, v5 = {5.0f}, v6 = {6.0f};
    v16sf vr = vector_reduction_10_operands(v1, v2, v3, v4, v5, v6);
    
    /* Test built-in complex math */
    double dr = complex_math_10_operands(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0);
    printf("Built-in math result: %f\n", dr);
    
    /* OpenMP SIMD reduction for additional coverage */
    #pragma omp simd reduction(+:dr)
    for (int i = 0; i < 100; i++) {
        dr += i * 0.1;
    }
    printf("OpenMP reduction result: %f\n", dr);
    
    return 0;
}
