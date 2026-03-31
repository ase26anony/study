/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
#include <emmintrin.h>
#else
/* Fallback definitions */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent dead code elimination */
static volatile int sink = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE static v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                       int m1, int m2, int m3, int m4) {
    /* This should expand to many operands during RTL generation */
#ifdef __SSE__
    __m128 va = _mm_load_ps((float*)&a);
    __m128 vb = _mm_load_ps((float*)&b);
    __m128 vc = _mm_load_ps((float*)&c);
    __m128 vd = _mm_load_ps((float*)&d);
    
    /* Complex shuffle pattern that may require many RTL operands */
    __m128 t1 = _mm_shuffle_ps(va, vb, m1);
    __m128 t2 = _mm_shuffle_ps(vc, vd, m2);
    __m128 t3 = _mm_shuffle_ps(t1, t2, m3);
    __m128 result = _mm_shuffle_ps(t3, _mm_setzero_ps(), m4);
    
    v4sf res;
    _mm_store_ps((float*)&res, result);
    return res;
#else
    /* Fallback for non-SSE */
    v4sf res = a + b + c + d;
    return res;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE static float pattern_b_fma_chain(float a, float b, float c, float d,
                                          float e, float f, float g, float h,
                                          float i, float j, float k, float l) {
    /* Chain of operations that may flatten to many operands */
#ifdef __FP_FAST_FMA
    /* Use builtin_fma if available */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    
    float t5 = __builtin_fmaf(t1, t2, t3);
    float result = __builtin_fmaf(t4, t5, t1 + t2 + t3 + t4);
#else
    /* Manual FMA simulation */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = j * k + l;
    
    float t5 = t1 * t2 + t3;
    float result = t4 * t5 + (t1 + t2 + t3 + t4);
#endif
    
    sink = (int)result; /* Prevent elimination */
    return result;
}

/* Pattern C: Vector extraction and horizontal sum with many operands */
NOINLINE static float pattern_c_vector_extract(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manual extraction of each element creates many operands */
    float sum = 0.0f;
    
    /* Extract and sum 16 elements (4 vectors * 4 elements) */
    for (int i = 0; i < 4; i++) {
#ifdef __SSE__
        sum += ((float*)&v1)[i] + ((float*)&v2)[i] + 
               ((float*)&v3)[i] + ((float*)&v4)[i];
#else
        sum += v1[i] + v2[i] + v3[i] + v4[i];
#endif
    }
    
    /* Additional arithmetic to increase operand count */
    sum = sum * 2.0f - sum / 2.0f + sum * 1.5f - sum / 1.5f;
    
    sink = (int)sum;
    return sum;
}

/* Pattern D: Vector conditional select with complex mask computation */
NOINLINE static v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                                  v4sf e, v4sf f) {
    /* Complex condition computation that may expand to many operands */
#ifdef __SSE__
    __m128 va = _mm_load_ps((float*)&a);
    __m128 vb = _mm_load_ps((float*)&b);
    __m128 vc = _mm_load_ps((float*)&c);
    __m128 vd = _mm_load_ps((float*)&d);
    __m128 ve = _mm_load_ps((float*)&e);
    __m128 vf = _mm_load_ps((float*)&f);
    
    /* Multiple comparisons and blends */
    __m128 cmp1 = _mm_cmplt_ps(va, vb);
    __m128 cmp2 = _mm_cmpgt_ps(vc, vd);
    __m128 cmp3 = _mm_cmpeq_ps(ve, vf);
    
    __m128 blend1 = _mm_blendv_ps(va, vb, cmp1);
    __m128 blend2 = _mm_blendv_ps(vc, vd, cmp2);
    __m128 blend3 = _mm_blendv_ps(ve, vf, cmp3);
    
    /* Final combination */
    __m128 result = _mm_add_ps(_mm_add_ps(blend1, blend2), blend3);
    
    v4sf res;
    _mm_store_ps((float*)&res, result);
    return res;
#else
    v4sf res = a + b + c + d + e + f;
    return res;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                                int f, int g, int h, int i, int j) {
    int result1, result2, result3;
    
    /* Inline assembly with many input/output operands */
    asm volatile (
        /* 11 operands total: 10 inputs + 1 output */
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r1], %[r1], %[d]\n\t"
        "add %[r2], %[e], %[f]\n\t"
        "add %[r2], %[r2], %[g]\n\t"
        "add %[r3], %[h], %[i]\n\t"
        "add %[r3], %[r3], %[j]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[out], %[r1], %[r3]"
        : [out] "=r" (result1), [r1] "=&r" (result2), [r2] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g),
          [h] "r" (h), [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    sink = result1;
    return result1;
}

/* Pattern F: ARM NEON specific with many operands */
#ifdef __ARM_NEON
NOINLINE static float32x4_t pattern_f_neon_ops(float32x4_t a, float32x4_t b,
                                               float32x4_t c, float32x4_t d,
                                               float32x4_t e, float32x4_t f) {
    /* Complex NEON operations that may require many RTL operands */
    float32x4_t t1 = vaddq_f32(a, b);
    float32x4_t t2 = vaddq_f32(c, d);
    float32x4_t t3 = vaddq_f32(e, f);
    
    float32x4_t t4 = vmulq_f32(t1, t2);
    float32x4_t t5 = vmlaq_f32(t4, t3, vdupq_n_f32(2.0f));
    
    float32x4_t mask = vcltq_f32(t5, vdupq_n_f32(0.0f));
    float32x4_t result = vbslq_f32(mask, t4, t5);
    
    return result;
}
#endif

/* Main test driver */
int main(int argc, char *argv[]) {
    float total = 0.0f;
    
    /* Initialize with some values */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf v5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf v6 = {21.0f, 22.0f, 23.0f, 24.0f};
    
    /* Use argc to select different patterns, ensuring all get compiled */
    switch (argc % 6) {
        case 0:
            total += pattern_a_shuffle(v1, v2, v3, v4, 0x1B, 0x27, 0x39, 0x4E)[0];
            break;
        case 1:
            total += pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
                                         7.7f, 8.8f, 9.9f, 10.1f, 11.1f, 12.1f);
            break;
        case 2:
            total += pattern_c_vector_extract(v1, v2, v3, v4);
            break;
        case 3:
            total += pattern_d_conditional_select(v1, v2, v3, v4, v5, v6)[0];
            break;
        case 4:
            total += (float)pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
        case 5:
#ifdef __ARM_NEON
            {
                float32x4_t a = {1.0f, 2.0f, 3.0f, 4.0f};
                float32x4_t b = {5.0f, 6.0f, 7.0f, 8.0f};
                float32x4_t c = {9.0f, 10.0f, 11.0f, 12.0f};
                float32x4_t d = {13.0f, 14.0f, 15.0f, 16.0f};
                float32x4_t e = {17.0f, 18.0f, 19.0f, 20.0f};
                float32x4_t f = {21.0f, 22.0f, 23.0f, 24.0f};
                float32x4_t res = pattern_f_neon_ops(a, b, c, d, e, f);
                total += vgetq_lane_f32(res, 0);
            }
#endif
            break;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    /* Also compute checksum to use all patterns */
    float checksum = 
        pattern_a_shuffle(v1, v2, v3, v4, 0x1B, 0x27, 0x39, 0x4E)[0] +
        pattern_b_fma_chain(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f) +
        pattern_c_vector_extract(v1, v2, v3, v4) +
        pattern_d_conditional_select(v1, v2, v3, v4, v5, v6)[0] +
        (float)pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    printf("Checksum: %f\n", checksum);
    
    return (int)checksum % 256;
}
