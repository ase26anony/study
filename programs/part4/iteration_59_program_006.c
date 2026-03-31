/* Test program to cover 10-11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

/* Force inline expansion */
#define FORCE_INLINE __attribute__((always_inline)) inline

/* Complex vector operation using AVX-512 intrinsics */
#ifdef __AVX512F__
FORCE_INLINE __m512 test_avx512_10_operands(__m512 a, __m512 b, __m512 c, 
                                           __m512 d, __m512 e, __mmask16 k) {
    /* This should generate a pattern with many operands:
       mask, src1, src2, src3, src4, src5, immediate, etc. */
    __m512 t1 = _mm512_mask_add_ps(a, k, b, c);
    __m512 t2 = _mm512_mask_mul_ps(t1, k, d, e);
    
    /* Nested FMA with mask - potentially expands to many operands */
    __m512 result = _mm512_mask_fmadd_ps(t2, k, 
                                        _mm512_set1_ps(2.0f),
                                        _mm512_set1_ps(3.0f));
    
    return result;
}

FORCE_INLINE __m512i test_avx512_11_operands(__m512i a, __m512i b, __m512i c,
                                            __m512i d, __m512i e, __m512i f,
                                            __mmask16 k1, __mmask16 k2) {
    /* Complex permute/shuffle with multiple masks and sources */
    __m512i t1 = _mm512_mask_add_epi32(a, k1, b, c);
    __m512i t2 = _mm512_mask_sub_epi32(d, k2, e, f);
    
    /* Blend with mask - adds more operands */
    __m512i result = _mm512_mask_blend_epi32(k1, t1, t2);
    
    /* Additional operation to ensure all operands are used */
    result = _mm512_mask_slli_epi32(result, k2, result, 3);
    
    return result;
}
#endif

/* GCC vector extensions for generic testing */
typedef float v16sf __attribute__((vector_size(64)));
typedef int v16si __attribute__((vector_size(64)));

FORCE_INLINE v16sf test_vector_ops_10(v16sf a, v16sf b, v16sf c, 
                                     v16sf d, v16sf e, v16sf f) {
    /* Complex expression that might expand to many operands */
    v16sf t1 = a + b * c;
    v16sf t2 = d - e / f;
    v16sf t3 = t1 * t2 + a;
    v16sf t4 = t3 - b + c;
    v16sf result = t4 * d + e - f;
    
    /* Add some conditional operations */
    v16sf mask = a > b;
    result = mask ? result * 2.0f : result / 2.0f;
    
    return result;
}

/* Inline assembly with many operands */
void test_asm_11_operands(void) {
    uint64_t a, b, c, d, e, f, g, h, i, j, k;
    
    /* Initialize to prevent optimization */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10; k = 11;
    
    /* 11-operand inline asm */
    __asm__ volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "mov %[k], %[a]"
        : [a] "+r" (a), [b] "+r" (b), [c] "+r" (c),
          [d] "+r" (d), [e] "+r" (e), [f] "+r" (f),
          [g] "+r" (g), [h] "+r" (h), [i] "+r" (i),
          [j] "+r" (j), [k] "+r" (k)
        :
        : "cc"
    );
    
    /* Use results to prevent dead code elimination */
    printf("ASM result: %lu\n", a + b + c + d + e + f + g + h + i + j + k);
}

/* OpenMP SIMD reduction with vector types */
#ifdef _OPENMP
void test_omp_reduction(void) {
    v16sf array[100];
    v16sf sum = {0};
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 16; j++) {
            array[i][j] = (float)(i + j);
        }
    }
    
    /* Complex reduction that might generate multi-operand patterns */
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum = sum + array[i] * array[i] - array[i] / 2.0f;
    }
    
    /* Use result */
    float total = 0;
    for (int i = 0; i < 16; i++) {
        total += sum[i];
    }
    printf("OMP reduction: %f\n", total);
}
#endif

/* Built-in math functions */
void test_builtin_fma(void) {
    double a = 1.5, b = 2.5, c = 3.5, d = 4.5, e = 5.5;
    
    /* Chain of FMA operations - each expands to 3 operands */
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(t1, d, e);
    double t3 = __builtin_fma(a, t2, b);
    double t4 = __builtin_fma(c, t3, d);
    double result = __builtin_fma(e, t4, a);
    
    /* Complex expression to prevent folding */
    result = result + __builtin_fma(result, 2.0, 3.0);
    
    printf("FMA result: %f\n", result);
}

/* Main test function */
int main(void) {
    printf("Testing multi-operand instruction patterns...\n");
    
    /* Test inline assembly */
    test_asm_11_operands();
    
    /* Test built-in FMA */
    test_builtin_fma();
    
#ifdef _OPENMP
    /* Test OpenMP reduction */
    test_omp_reduction();
#endif
    
#ifdef __AVX512F__
    /* Test AVX-512 intrinsics */
    __m512 vec1 = _mm512_set1_ps(1.0f);
    __m512 vec2 = _mm512_set1_ps(2.0f);
    __m512 vec3 = _mm512_set1_ps(3.0f);
    __m512 vec4 = _mm512_set1_ps(4.0f);
    __m512 vec5 = _mm512_set1_ps(5.0f);
    __mmask16 mask = 0xAAAA;
    
    __m512 res1 = test_avx512_10_operands(vec1, vec2, vec3, vec4, vec5, mask);
    
    __m512i ivec1 = _mm512_set1_epi32(1);
    __m512i ivec2 = _mm512_set1_epi32(2);
    __m512i ivec3 = _mm512_set1_epi32(3);
    __m512i ivec4 = _mm512_set1_epi32(4);
    __m512i ivec5 = _mm512_set1_epi32(5);
    __m512i ivec6 = _mm512_set1_epi32(6);
    __mmask16 mask2 = 0x5555;
    
    __m512i res2 = test_avx512_11_operands(ivec1, ivec2, ivec3, 
                                          ivec4, ivec5, ivec6,
                                          mask, mask2);
    
    /* Use results to prevent optimization */
    float sum1 = 0;
    int sum2 = 0;
    for (int i = 0; i < 16; i++) {
        sum1 += res1[i];
        sum2 += res2[i];
    }
    printf("AVX-512 results: %f, %d\n", sum1, sum2);
#endif
    
    /* Test GCC vector extensions */
    v16sf v1 = {0}, v2 = {0}, v3 = {0}, v4 = {0}, v5 = {0}, v6 = {0};
    for (int i = 0; i < 16; i++) {
        v1[i] = i * 1.0f;
        v2[i] = i * 2.0f;
        v3[i] = i * 3.0f;
        v4[i] = i * 4.0f;
        v5[i] = i * 5.0f;
        v6[i] = i * 6.0f;
    }
    
    v16sf vres = test_vector_ops_10(v1, v2, v3, v4, v5, v6);
    
    float total = 0;
    for (int i = 0; i < 16; i++) {
        total += vres[i];
    }
    printf("Vector extension result: %f\n", total);
    
    return 0;
}
