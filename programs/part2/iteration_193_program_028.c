/* test_optabs.c - Coverage test for 10/11 operand switch cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for portability */
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#endif

#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>
typedef float32x4_t v4sf;
typedef int32x4_t v4si;
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE static v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                       int imm1, int imm2, int imm3, int imm4) {
#if defined(__SSE__)
    /* This creates many operands: 4 vectors + 4 immediates + temporaries */
    v4sf t1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf t2 = __builtin_ia32_shufps(c, d, imm2);
    v4sf t3 = __builtin_ia32_shufps(t1, t2, imm3);
    return __builtin_ia32_shufps(t3, a, imm4);
#elif defined(__ARM_NEON)
    /* ARM equivalent with multiple operations */
    v4sf t1 = vrev64q_f32(a);
    v4sf t2 = vrev64q_f32(b);
    v4sf t3 = vzip1q_f32(c, d);
    v4sf t4 = vzip2q_f32(t1, t2);
    return vaddq_f32(t3, t4);
#else
    /* Fallback for testing */
    return a;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE static float pattern_b_fma_chain(float a, float b, float c, float d,
                                          float e, float f, float g, float h,
                                          float i, float j, float k, float l) {
#if defined(__FMA__) || defined(__AVX2__)
    /* Chain of FMA operations that may expand to many operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    
    /* Create dependencies to prevent reordering */
    float r1 = __builtin_fmaf(t1, t2, t3);
    float r2 = __builtin_fmaf(t4, t1, t2);
    return __builtin_fmaf(r1, r2, t4);
#else
    /* Manual FMA simulation */
    return a * b + c * d + e * f + g * h + i * j + k * l;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Extract each element - creates many extract operations */
    float sum = 0.0f;
    
#if defined(__SSE__)
    /* Each extract adds operands */
    sum += ((float*)&v1)[0] + ((float*)&v1)[1] + ((float*)&v1)[2] + ((float*)&v1)[3];
    sum += ((float*)&v2)[0] + ((float*)&v2)[1] + ((float*)&v2)[2] + ((float*)&v2)[3];
    sum += ((float*)&v3)[0] + ((float*)&v3)[1] + ((float*)&v3)[2] + ((float*)&v3)[3];
    sum += ((float*)&v4)[0] + ((float*)&v4)[1] + ((float*)&v4)[2] + ((float*)&v4)[3];
#elif defined(__ARM_NEON)
    float32x4_t t1 = vaddq_f32(v1, v2);
    float32x4_t t2 = vaddq_f32(v3, v4);
    float32x4_t t3 = vaddq_f32(t1, t2);
    sum = vaddvq_f32(t3);
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE static v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                                  v4sf mask1, v4sf mask2) {
#if defined(__SSE__)
    /* Multiple comparisons and blends */
    v4sf cmp1 = __builtin_ia32_cmpps(a, b, 1);  /* LT */
    v4sf cmp2 = __builtin_ia32_cmpps(c, d, 1);  /* LT */
    v4sf mask = __builtin_ia32_andps(cmp1, mask1);
    mask = __builtin_ia32_orps(mask, mask2);
    
    /* Blend based on complex mask - may expand to many operands */
    v4sf t1 = __builtin_ia32_andps(a, mask);
    v4sf t2 = __builtin_ia32_andnps(mask, b);
    return __builtin_ia32_orps(t1, t2);
#elif defined(__ARM_NEON)
    v4sf cmp1 = vcltq_f32(a, b);
    v4sf cmp2 = vcltq_f32(c, d);
    v4sf mask = vandq_f32(cmp1, mask1);
    mask = vorrq_f32(mask, mask2);
    return vbslq_f32(mask, a, b);
#else
    return a;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                                int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline assembly with 10 input operands + 1 output = 11 total */
    __asm__ volatile (
        "add %[a], %[b], %[tmp1]\n\t"
        "add %[c], %[d], %[tmp2]\n\t"
        "add %[e], %[f], %[tmp3]\n\t"
        "add %[g], %[h], %[tmp4]\n\t"
        "add %[i], %[j], %[tmp5]\n\t"
        "add %[tmp1], %[tmp2], %[tmp6]\n\t"
        "add %[tmp3], %[tmp4], %[tmp7]\n\t"
        "add %[tmp5], %[tmp6], %[tmp8]\n\t"
        "add %[tmp7], %[tmp8], %[out]"
        : [out] "=r" (result),
          [tmp1] "=&r" (a), [tmp2] "=&r" (b), [tmp3] "=&r" (c),
          [tmp4] "=&r" (d), [tmp5] "=&r" (e), [tmp6] "=&r" (f),
          [tmp7] "=&r" (g), [tmp8] "=&r" (h)
        : [a] "0" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}

/* Wrapper that combines multiple patterns to ensure coverage */
NOINLINE static float test_all_patterns(int argc, char** argv) {
    volatile float checksum = 0.0f;
    
    /* Initialize with some data */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf mask1 = {0.0f, 1.0f, 0.0f, 1.0f};
    v4sf mask2 = {1.0f, 0.0f, 1.0f, 0.0f};
    
    /* Use argc to select different patterns, ensuring all get compiled */
    switch (argc % 5) {
        case 0:
            /* Pattern A: Complex shuffle */
            checksum += ((float*)&pattern_a_shuffle(v1, v2, v3, v4, 0x1B, 0x27, 0x39, 0x4E))[0];
            break;
        case 1:
            /* Pattern B: FMA chain */
            checksum += pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
                                           7.7f, 8.8f, 9.9f, 10.1f, 11.1f, 12.1f);
            break;
        case 2:
            /* Pattern C: Vector reduction */
            checksum += pattern_c_vector_reduce(v1, v2, v3, v4);
            break;
        case 3:
            /* Pattern D: Conditional select */
            checksum += ((float*)&pattern_d_conditional_select(v1, v2, v3, v4, mask1, mask2))[0];
            break;
        case 4:
            /* Pattern E: Multi-operand assembly */
            checksum += pattern_e_multi_operand_asm(argc, argc+1, argc+2, argc+3, argc+4,
                                                   argc+5, argc+6, argc+7, argc+8, argc+9);
            break;
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    float result = 0.0f;
    
    /* Run multiple iterations to ensure all paths are taken */
    for (int i = 0; i < 10; i++) {
        result += test_all_patterns(argc + i, argv);
    }
    
    /* Use result to prevent optimization */
    volatile float* volatile_ptr = &result;
    printf("Result: %f\n", *volatile_ptr);
    
    return (int)result;
}

/* Additional test: Direct built-in with many arguments for x86 */
#if defined(__AVX512F__)
NOINLINE static __m512 pattern_avx512_multi_operand(__m512 a, __m512 b, __m512 c,
                                                    __m512 d, __m512 e, __m512 f,
                                                    int imm1, int imm2, int imm3) {
    /* AVX-512 operations often expand to many operands */
    __m512 t1 = _mm512_fmadd_ps(a, b, c);
    __m512 t2 = _mm512_fmadd_ps(d, e, f);
    __m512 mask = _mm512_cmp_ps_mask(t1, t2, _CMP_LT_OQ);
    return _mm512_mask_blend_ps(mask, t1, t2);
}
#endif
