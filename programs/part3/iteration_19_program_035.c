/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Target-specific multi-operand operations */

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

/* Complex inline assembly with many operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, uint64_t c,
                                               uint64_t d, uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h, uint64_t i) {
    uint64_t result1, result2, result3;
    
    /* 10-operand asm: 9 inputs + 1 output */
    asm volatile (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "imul %[c], %[r1]\n\t"
        "add %[d], %[r1]\n\t"
        "sub %[e], %[r1]\n\t"
        "xor %[f], %[r1]\n\t"
        "or %[g], %[r1]\n\t"
        "and %[h], %[r1]\n\t"
        "add %[i], %[r1]"
        : [r1] "=&r" (result1)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    /* Another variant with clobbers */
    asm volatile (
        "lea (%[a],%[b],1), %[r2]\n\t"
        "lea (%[r2],%[c],2), %[r2]\n\t"
        "lea (%[r2],%[d],4), %[r2]\n\t"
        "lea (%[r2],%[e],8), %[r2]"
        : [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e)
        : "cc"
    );
    
    return result1 + result2;
}

/* AVX-512 operations that might expand to many operands */
NOINLINE static __m512i avx512_complex_op(__m512i a, __m512i b, __m512i c,
                                          __m512i d, __m512i e) {
    /* FMA chain that might require many operands */
    __m512i t1 = _mm512_mullo_epi32(a, b);
    __m512i t2 = _mm512_mullo_epi32(c, d);
    __m512i t3 = _mm512_add_epi32(t1, t2);
    __m512i t4 = _mm512_mullo_epi32(t3, e);
    __m512i t5 = _mm512_slli_epi32(t4, 3);
    __m512i t6 = _mm512_srli_epi32(t5, 1);
    
    return _mm512_xor_si512(t6, _mm512_set1_epi32(0x55555555));
}

#endif /* __x86_64__ */

#ifdef __aarch64__
#include <arm_neon.h>

/* ARM NEON operations with lane selection */
NOINLINE static int32x4_t aarch64_multi_lane_op(int32x4_t a, int32x4_t b,
                                                int32x4_t c, int32x4_t d,
                                                int32_t e, int32_t f,
                                                int32_t g, int32_t h) {
    /* Complex sequence that might expand to many operands */
    int32x4_t t1 = vmlaq_s32(a, b, c);
    int32x4_t t2 = vmlsq_s32(d, t1, vdupq_n_s32(e));
    int32x4_t t3 = vqdmulhq_s32(t2, vdupq_n_s32(f));
    int32x4_t t4 = vqrshlq_s32(t3, vdupq_n_s32(g));
    
    return vaddq_s32(t4, vdupq_n_s32(h));
}

#endif /* __aarch64__ */

/* Multi-precision arithmetic that might trigger expand_mult_highpart */
NOINLINE static uint64_t multi_precision_mul(uint64_t a, uint64_t b,
                                             uint64_t c, uint64_t d) {
    /* Complex expression that GCC might break into many operations */
    uint64_t t1 = (a * b) >> 32;      /* high part multiplication */
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (a * c) & 0xFFFFFFFF;
    uint64_t t4 = (b * d) & 0xFFFFFFFF;
    
    /* Chain of operations that might require many operands */
    uint64_t result = t1 + t2;
    result = (result << 16) | (t3 >> 16);
    result = result ^ (t4 << 16);
    result = result * 0x9E3779B97F4A7C15ULL;
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE static v4si vector_complex_op(v4si a, v4si b, v4si c, v4si d,
                                       v4si e, v4si f) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e - f;
    v4si t3 = (t1 >> 4) | (t2 << 4);
    v4si t4 = (a & b) | (c & d) | (e & f);
    
    return t3 ^ t4;
}

/* Function with many arithmetic operations in one expression */
NOINLINE static int many_operand_expr(int a, int b, int c, int d, int e,
                                      int f, int g, int h, int i, int j) {
    /* Single expression with 10 operands - might trigger special expansion */
    int result = ((a * b) + (c * d) - (e * f)) / ((g + h) * (i - j));
    result = result * ((a + b + c + d + e + f + g + h + i + j) & 0xFF);
    
    /* Additional complex operation */
    result = (result << 3) | (result >> 29);
    result = result ^ 0xAAAAAAAA;
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t total = 0;
    
    /* Initialize test values based on argc for variability */
    uint64_t a = argc + 1;
    uint64_t b = argc + 2;
    uint64_t c = argc + 3;
    uint64_t d = argc + 4;
    uint64_t e = argc + 5;
    uint64_t f = argc + 6;
    uint64_t g = argc + 7;
    uint64_t h = argc + 8;
    uint64_t i = argc + 9;
    uint64_t j = argc + 10;
    
    /* Test different code paths based on command line */
    if (argc > 1) {
        /* Path 1: Multi-precision arithmetic */
        for (int iter = 0; iter < 100; iter++) {
            total += multi_precision_mul(a + iter, b + iter, 
                                        c + iter, d + iter);
        }
    } else {
        /* Path 2: Many operand expression */
        for (int iter = 0; iter < 100; iter++) {
            total += many_operand_expr(a + iter, b + iter, c + iter,
                                      d + iter, e + iter, f + iter,
                                      g + iter, h + iter, i + iter,
                                      j + iter);
        }
    }
    
    /* Target-specific operations */
#ifdef __x86_64__
    if (argc > 2) {
        total += x86_multi_operand_asm(a, b, c, d, e, f, g, h, i);
    }
    
    /* Test with AVX-512 if available */
    #ifdef __AVX512F__
    if (argc > 3) {
        __m512i vec_a = _mm512_set1_epi32(a);
        __m512i vec_b = _mm512_set1_epi32(b);
        __m512i vec_c = _mm512_set1_epi32(c);
        __m512i vec_d = _mm512_set1_epi32(d);
        __m512i vec_e = _mm512_set1_epi32(e);
        
        __m512i vec_result = avx512_complex_op(vec_a, vec_b, vec_c, vec_d, vec_e);
        alignas(64) int32_t result_arr[16];
        _mm512_store_si512(result_arr, vec_result);
        
        for (int k = 0; k < 16; k++) {
            total += result_arr[k];
        }
    }
    #endif
#endif /* __x86_64__ */
    
#ifdef __aarch64__
    if (argc > 4) {
        int32x4_t vec_a = vdupq_n_s32(a);
        int32x4_t vec_b = vdupq_n_s32(b);
        int32x4_t vec_c = vdupq_n_s32(c);
        int32x4_t vec_d = vdupq_n_s32(d);
        
        int32x4_t vec_result = aarch64_multi_lane_op(vec_a, vec_b, vec_c, vec_d,
                                                    e, f, g, h);
        int32_t result_arr[4];
        vst1q_s32(result_arr, vec_result);
        
        for (int k = 0; k < 4; k++) {
            total += result_arr[k];
        }
    }
#endif /* __aarch64__ */
    
    /* Vector extension test */
    v4si vec1 = {a, b, c, d};
    v4si vec2 = {e, f, g, h};
    v4si vec3 = {i, j, a, b};
    v4si vec4 = {c, d, e, f};
    v4si vec5 = {g, h, i, j};
    v4si vec6 = {a, c, e, g};
    
    v4si vec_result = vector_complex_op(vec1, vec2, vec3, vec4, vec5, vec6);
    for (int k = 0; k < 4; k++) {
        total += vec_result[k];
    }
    
    /* Prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)total);
    
    return (int)(total & 0x7FFFFFFF);
}
