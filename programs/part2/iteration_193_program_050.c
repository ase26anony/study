/* test_optabs_high_operand.c
 * 
 * This test targets uncovered switch cases in optabs.cc (lines 8254-8263)
 * that handle operations with 10 and 11 operands during RTL expansion.
 * 
 * Compile with: gcc -O3 -march=native -fno-tree-vectorize -fprofile-arcs -ftest-coverage -o test_optabs test_optabs_high_operand.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate expansion contexts */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Vector type definitions */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* SSE intrinsics if available */
#ifdef __SSE__
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

#ifdef __SSE3__
#include <pmmintrin.h>
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

#ifdef __AVX__
#include <immintrin.h>
#endif

/* ARM NEON intrinsics if available */
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Global volatile to prevent optimization */
static volatile int g_checksum = 0;

/* Pattern A: Vector blend with complex mask computation (targets ~10 operands) */
NOINLINE v4sf pattern_a_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                                      int m1, int m2, int m3, int m4) {
#ifdef __SSE4_1__
    /* Create complex mask from multiple conditions */
    v4sf cmp1 = _mm_cmpgt_ps(a, b);      /* 4 comparisons */
    v4sf cmp2 = _mm_cmplt_ps(c, d);      /* 4 comparisons */
    v4sf mask = _mm_and_ps(cmp1, cmp2);  /* 4 AND operations */
    
    /* Blend using the computed mask - this may expand to many operands */
    v4sf result = _mm_blendv_ps(a, b, mask);
    
    /* Additional operations to increase operand count */
    result = _mm_add_ps(result, c);
    result = _mm_sub_ps(result, d);
    
    return result;
#elif defined(__ARM_NEON)
    /* ARM NEON equivalent */
    float32x4_t cmp1 = vcgtq_f32((float32x4_t)a, (float32x4_t)b);
    float32x4_t cmp2 = vcltq_f32((float32x4_t)c, (float32x4_t)d);
    uint32x4_t mask = vandq_u32((uint32x4_t)cmp1, (uint32x4_t)cmp2);
    
    float32x4_t result = vbslq_f32(mask, (float32x4_t)a, (float32x4_t)b);
    result = vaddq_f32(result, (float32x4_t)c);
    result = vsubq_f32(result, (float32x4_t)d);
    
    return (v4sf)result;
#else
    /* Fallback scalar implementation */
    v4sf result = a;
    for (int i = 0; i < 4; i++) {
        float mask_val = (a[i] > b[i] && c[i] < d[i]) ? -1.0f : 0.0f;
        result[i] = (mask_val != 0.0f) ? a[i] : b[i];
        result[i] += c[i];
        result[i] -= d[i];
    }
    return result;
#endif
}

/* Pattern B: Fused multiply-add chain (deep expression tree -> many operands) */
NOINLINE v4sf pattern_b_fma_chain(v4sf a, v4sf b, v4sf c, v4sf d, 
                                  v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __FMA__
    /* Chain of FMA operations creating deep expression tree */
    v4sf t1 = __builtin_fma(a, b, c);    /* 3 operands */
    v4sf t2 = __builtin_fma(d, e, f);    /* 3 operands */
    v4sf t3 = __builtin_fma(g, h, t1);   /* 3 operands + t1 */
    v4sf result = __builtin_fma(t2, t3, a + b + c + d); /* Many operands */
    
    return result;
#elif defined(__SSE3__)
    /* Manual FMA simulation */
    v4sf t1 = _mm_mul_ps(a, b);
    t1 = _mm_add_ps(t1, c);
    
    v4sf t2 = _mm_mul_ps(d, e);
    t2 = _mm_add_ps(t2, f);
    
    v4sf t3 = _mm_mul_ps(g, h);
    t3 = _mm_add_ps(t3, t1);
    
    v4sf sum = _mm_add_ps(a, b);
    sum = _mm_add_ps(sum, c);
    sum = _mm_add_ps(sum, d);
    
    v4sf result = _mm_mul_ps(t2, t3);
    result = _mm_add_ps(result, sum);
    
    return result;
#else
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float t1 = a[i] * b[i] + c[i];
        float t2 = d[i] * e[i] + f[i];
        float t3 = g[i] * h[i] + t1;
        float sum = a[i] + b[i] + c[i] + d[i];
        result[i] = t2 * t3 + sum;
    }
    return result;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization (many extract operations) */
NOINLINE float pattern_c_scalarized_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    float sum = 0.0f;
    
    /* Manually extract and sum all elements - each extract is an operand */
#ifdef __SSE__
    /* Use SSE extract operations */
    sum += _mm_cvtss_f32(v1);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 3, 3, 3)));
    
    sum += _mm_cvtss_f32(v2);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 3, 3, 3)));
    
    sum += _mm_cvtss_f32(v3);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v3, v3, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v3, v3, _MM_SHUFFLE(3, 3, 3, 3)));
    
    sum += _mm_cvtss_f32(v4);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v4, v4, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v4, v4, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v4, v4, _MM_SHUFFLE(3, 3, 3, 3)));
#else
    /* Scalar fallback */
    for (int i = 0; i < 4; i++) sum += v1[i];
    for (int i = 0; i < 4; i++) sum += v2[i];
    for (int i = 0; i < 4; i++) sum += v3[i];
    for (int i = 0; i < 4; i++) sum += v4[i];
#endif
    
    return sum;
}

/* Pattern D: Complex conditional vector operations */
NOINLINE v4sf pattern_d_conditional_merge(v4sf a, v4sf b, v4sf c, v4sf d,
                                          v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE__
    /* Multiple comparisons and blends */
    v4sf cmp1 = _mm_cmpgt_ps(a, b);
    v4sf cmp2 = _mm_cmplt_ps(c, d);
    v4sf cmp3 = _mm_cmpeq_ps(e, f);
    
    /* Combine masks with logical operations */
    v4sf mask1 = _mm_and_ps(cmp1, cmp2);
    v4sf mask2 = _mm_or_ps(mask1, cmp3);
    v4sf mask3 = _mm_andnot_ps(cmp1, cmp2);
    
    /* Multiple blend operations */
    v4sf temp1 = _mm_blendv_ps(a, b, mask1);
    v4sf temp2 = _mm_blendv_ps(c, d, mask2);
    v4sf temp3 = _mm_blendv_ps(e, f, mask3);
    
    /* Final combination */
    v4sf result = _mm_add_ps(temp1, temp2);
    result = _mm_sub_ps(result, temp3);
    result = _mm_mul_ps(result, g);
    result = _mm_div_ps(result, h);
    
    return result;
#else
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float mask1_val = (a[i] > b[i] && c[i] < d[i]) ? -1.0f : 0.0f;
        float mask2_val = (mask1_val != 0.0f || e[i] == f[i]) ? -1.0f : 0.0f;
        float mask3_val = (!(a[i] > b[i]) && c[i] < d[i]) ? -1.0f : 0.0f;
        
        float temp1 = (mask1_val != 0.0f) ? a[i] : b[i];
        float temp2 = (mask2_val != 0.0f) ? c[i] : d[i];
        float temp3 = (mask3_val != 0.0f) ? e[i] : f[i];
        
        result[i] = (temp1 + temp2 - temp3) * g[i] / h[i];
    }
    return result;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE int64_t pattern_e_asm_11_operands(int64_t a, int64_t b, int64_t c,
                                          int64_t d, int64_t e, int64_t f,
                                          int64_t g, int64_t h, int64_t i,
                                          int64_t j, int64_t k) {
    int64_t result;
    
    /* Inline assembly with 11 operands (10 inputs + 1 output) */
    __asm__ volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "imul %[k], %[result]\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

/* Pattern F: Shuffle with complex immediate computation */
NOINLINE v4sf pattern_f_complex_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                                        int imm1, int imm2, int imm3, int imm4) {
#ifdef __SSE__
    /* Multiple shuffle operations with computed immediates */
    v4sf s1 = _mm_shuffle_ps(a, b, imm1);
    v4sf s2 = _mm_shuffle_ps(c, d, imm2);
    v4sf s3 = _mm_shuffle_ps(s1, s2, imm3);
    v4sf s4 = _mm_shuffle_ps(b, c, imm4);
    
    /* Combine with arithmetic */
    v4sf t1 = _mm_add_ps(s1, s2);
    v4sf t2 = _mm_sub_ps(s3, s4);
    v4sf result = _mm_mul_ps(t1, t2);
    
    /* Additional shuffle to increase complexity */
    result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 3, 0, 1));
    
    return result;
#else
    v4sf result;
    for (int i = 0; i < 4; i++) {
        int idx1 = (imm1 >> (i * 2)) & 0x3;
        int idx2 = (imm2 >> (i * 2)) & 0x3;
        float val1 = (idx1 < 2) ? a[idx1] : b[idx1 - 2];
        float val2 = (idx2 < 2) ? c[idx2] : d[idx2 - 2];
        result[i] = val1 + val2;
    }
    return result;
#endif
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize with some data */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf v5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf v6 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf v7 = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf v8 = {29.0f, 30.0f, 31.0f, 32.0f};
    
    int checksum = 0;
    
    /* Use argc to vary which patterns are executed */
    int pattern_selector = argc > 1 ? atoi(argv[1]) % 6 : 0;
    
    switch (pattern_selector) {
        case 0:
            /* Pattern A: Complex blend */
            {
                v4sf result = pattern_a_blend_complex(v1, v2, v3, v4, 1, 2, 3, 4);
                for (int i = 0; i < 4; i++) checksum += (int)result[i];
            }
            break;
            
        case 1:
            /* Pattern B: FMA chain */
            {
                v4sf result = pattern_b_fma_chain(v1, v2, v3, v4, v5, v6, v7, v8);
                for (int i = 0; i < 4; i++) checksum += (int)result[i];
            }
            break;
            
        case 2:
            /* Pattern C: Scalarized reduction */
            {
                float result = pattern_c_scalarized_reduction(v1, v2, v3, v4);
                checksum += (int)result;
            }
            break;
            
        case 3:
            /* Pattern D: Conditional merge */
            {
                v4sf result = pattern_d_conditional_merge(v1, v2, v3, v4, v5, v6, v7, v8);
                for (int i = 0; i < 4; i++) checksum += (int)result[i];
            }
            break;
            
        case 4:
            /* Pattern E: Inline assembly with 11 operands */
            {
                int64_t result = pattern_e_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
                checksum += (int)result;
            }
            break;
            
        case 5:
            /* Pattern F: Complex shuffle */
            {
                v4sf result = pattern_f_complex_shuffle(v1, v2, v3, v4, 
                                                       _MM_SHUFFLE(0, 1, 2, 3),
                                                       _MM_SHUFFLE(3, 2, 1, 0),
                                                       _MM_SHUFFLE(1, 0, 3, 2),
                                                       _MM_SHUFFLE(2, 3, 0, 1));
                for (int i = 0; i < 4; i++) checksum += (int)result[i];
            }
            break;
    }
    
    /* Additional test: Call all patterns to ensure compilation coverage */
    if (argc > 2) {
        /* Force compilation of all patterns */
        v4sf r1 = pattern_a_blend_complex(v1, v2, v3, v4, 1, 2, 3, 4);
        v4sf r2 = pattern_b_fma_chain(v1, v2, v3, v4, v5, v6, v7, v8);
        float r3 = pattern_c_scalarized_reduction(v1, v2, v3, v4);
        v4sf r4 = pattern_d_conditional_merge(v1, v2, v3, v4, v5, v6, v7, v8);
        int64_t r5 = pattern_e_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        v4sf r6 = pattern_f_complex_shuffle(v1, v2, v3, v4, 0x1B, 0x27, 0x39, 0x4E);
        
        checksum += (int)r1[0] + (int)r2[0] + (int)r3 + (int)r4[0] + (int)r5 + (int)r6[0];
    }
    
    /* Store to volatile global to prevent optimization */
    g_checksum = checksum;
    
    printf("Checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
