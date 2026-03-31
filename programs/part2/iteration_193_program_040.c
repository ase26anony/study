/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations that might eliminate our test patterns */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Volatile sink to prevent optimization */
static volatile int sink = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE v4sf pattern_a_shuffle(v4sf a, v4sf b, int mask0, int mask1, 
                                int mask2, int mask3, float s0, float s1) {
#ifdef __SSE__
    /* Use SSE shuffle intrinsic - expands to multiple RTL operands */
    v4sf result = __builtin_ia32_shufps(a, b, 
        (mask0 & 3) | ((mask1 & 3) << 2) | ((mask2 & 3) << 4) | ((mask3 & 3) << 6));
    
    /* Additional operations to increase operand count */
    result += (v4sf){s0, s1, s0, s1};
    result = __builtin_ia32_shufps(result, result, 0x1B); /* Another shuffle */
    
    return result;
#else
    /* Fallback for non-SSE targets */
    return a + b + (v4sf){s0, s1, s0, s1};
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE float pattern_b_fma_chain(float a, float b, float c, float d,
                                   float e, float f, float g, float h) {
#ifdef __FMA__
    /* Chain of FMA operations - expands to many RTL operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, t1);
    float t4 = __builtin_fmaf(t1, t2, t3);
    float t5 = __builtin_fmaf(t2, t3, t4);
    float t6 = __builtin_fmaf(t3, t4, t5);
    return __builtin_fmaf(t4, t5, t6);
#else
    /* Manual FMA simulation */
    return a * b + c + d * e + f + g * h;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE float pattern_c_vector_reduce(v4sf v, v4sf w, v4sf x, v4sf y) {
    float sum = 0.0f;
    
    /* Extract and sum each element - creates many extract operations */
#ifdef __SSE__
    sum += __builtin_ia32_vec_ext_v4sf(v, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v, 3);
    
    sum += __builtin_ia32_vec_ext_v4sf(w, 0);
    sum += __builtin_ia32_vec_ext_v4sf(w, 1);
    sum += __builtin_ia32_vec_ext_v4sf(w, 2);
    sum += __builtin_ia32_vec_ext_v4sf(w, 3);
    
    sum += __builtin_ia32_vec_ext_v4sf(x, 0);
    sum += __builtin_ia32_vec_ext_v4sf(x, 1);
    sum += __builtin_ia32_vec_ext_v4sf(x, 2);
    sum += __builtin_ia32_vec_ext_v4sf(x, 3);
    
    sum += __builtin_ia32_vec_ext_v4sf(y, 0);
    sum += __builtin_ia32_vec_ext_v4sf(y, 1);
    sum += __builtin_ia32_vec_ext_v4sf(y, 2);
    sum += __builtin_ia32_vec_ext_v4sf(y, 3);
#else
    /* Fallback for non-SSE */
    float* vp = (float*)&v;
    float* wp = (float*)&w;
    float* xp = (float*)&x;
    float* yp = (float*)&y;
    
    for (int i = 0; i < 4; i++) {
        sum += vp[i] + wp[i] + xp[i] + yp[i];
    }
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, 
                                           v4sf d, v4sf e, v4sf f) {
#ifdef __SSE__
    /* Multiple comparisons and blends */
    v4sf cmp1 = __builtin_ia32_cmpps(a, b, 1);  /* LT comparison */
    v4sf cmp2 = __builtin_ia32_cmpps(c, d, 1);  /* Another comparison */
    v4sf cmp3 = __builtin_ia32_cmpps(e, f, 1);  /* Third comparison */
    
    /* Combine masks with logical operations */
    v4sf mask1 = __builtin_ia32_andps(cmp1, cmp2);
    v4sf mask2 = __builtin_ia32_orps(cmp1, cmp3);
    v4sf mask3 = __builtin_ia32_andnps(mask1, mask2);
    
    /* Blend based on complex mask */
    v4sf result = __builtin_ia32_blendvps(a, b, mask1);
    result = __builtin_ia32_blendvps(result, c, mask2);
    result = __builtin_ia32_blendvps(result, d, mask3);
    
    return result;
#else
    /* Fallback */
    return a + b + c + d + e + f;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE int64_t pattern_e_multi_operand_asm(int64_t a, int64_t b, int64_t c,
                                             int64_t d, int64_t e, int64_t f,
                                             int64_t g, int64_t h, int64_t i,
                                             int64_t j) {
    int64_t result1, result2;
    
    /* Inline assembly with 11 total operands (2 outputs, 9 inputs) */
    __asm__ volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r1], %[r1], %[d]\n\t"
        "add %[r2], %[e], %[f]\n\t"
        "add %[r2], %[r2], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r1], %[r1], %[j]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1;
}

/* Pattern F: ARM NEON specific - multiple vector operations */
NOINLINE float32x4_t pattern_f_neon_ops(float32x4_t a, float32x4_t b,
                                        float32x4_t c, float32x4_t d,
                                        float32x4_t e, float32x4_t f) {
#ifdef __ARM_NEON
    /* Complex sequence of NEON operations */
    float32x4_t t1 = vaddq_f32(a, b);
    float32x4_t t2 = vmulq_f32(c, d);
    float32x4_t t3 = vmlaq_f32(t1, e, f);  /* FMA: t1 + e * f */
    
    /* Lane extractions and inserts */
    float32_t lane0 = vgetq_lane_f32(t1, 0);
    float32_t lane1 = vgetq_lane_f32(t2, 1);
    float32_t lane2 = vgetq_lane_f32(t3, 2);
    
    t1 = vsetq_lane_f32(lane0 + lane1, t1, 0);
    t2 = vsetq_lane_f32(lane1 * lane2, t2, 1);
    t3 = vsetq_lane_f32(lane2 - lane0, t3, 2);
    
    /* Final blend-like operation */
    uint32x4_t mask = vcltq_f32(t1, t2);
    return vbslq_f32(mask, t1, t3);
#else
    /* Fallback for non-NEON */
    return a + b + c + d + e + f;
#endif
}

/* Main test driver */
int main(int argc, char** argv) {
    int checksum = 0;
    
    /* Initialize with some values, using argc for variability */
    float f1 = argc * 1.1f;
    float f2 = argc * 2.2f;
    float f3 = argc * 3.3f;
    float f4 = argc * 4.4f;
    
    v4sf v1 = {f1, f2, f3, f4};
    v4sf v2 = {f2, f3, f4, f1};
    v4sf v3 = {f3, f4, f1, f2};
    v4sf v4 = {f4, f1, f2, f3};
    v4sf v5 = {f1, f3, f2, f4};
    v4sf v6 = {f2, f4, f1, f3};
    
    /* Execute all patterns to ensure compilation and coverage */
    
    /* Pattern A: Shuffle with many operands */
    v4sf res_a = pattern_a_shuffle(v1, v2, argc, argc+1, argc+2, argc+3, f1, f2);
    checksum += (int)(res_a[0] + res_a[1] + res_a[2] + res_a[3]);
    
    /* Pattern B: FMA chain */
    float res_b = pattern_b_fma_chain(f1, f2, f3, f4, f1, f2, f3, f4);
    checksum += (int)res_b;
    
    /* Pattern C: Vector reduction */
    float res_c = pattern_c_vector_reduce(v1, v2, v3, v4);
    checksum += (int)res_c;
    
    /* Pattern D: Conditional select */
    v4sf res_d = pattern_d_conditional_select(v1, v2, v3, v4, v5, v6);
    checksum += (int)(res_d[0] + res_d[1] + res_d[2] + res_d[3]);
    
    /* Pattern E: Multi-operand assembly */
    int64_t res_e = pattern_e_multi_operand_asm(argc, argc+1, argc+2, argc+3,
                                                argc+4, argc+5, argc+6,
                                                argc+7, argc+8, argc+9);
    checksum += (int)res_e;
    
#ifdef __ARM_NEON
    /* Pattern F: NEON operations */
    float32x4_t a = {f1, f2, f3, f4};
    float32x4_t b = {f2, f3, f4, f1};
    float32x4_t c = {f3, f4, f1, f2};
    float32x4_t d = {f4, f1, f2, f3};
    float32x4_t e = {f1, f3, f2, f4};
    float32x4_t f = {f2, f4, f1, f3};
    
    float32x4_t res_f = pattern_f_neon_ops(a, b, c, d, e, f);
    checksum += (int)(vgetq_lane_f32(res_f, 0) +
                      vgetq_lane_f32(res_f, 1) +
                      vgetq_lane_f32(res_f, 2) +
                      vgetq_lane_f32(res_f, 3));
#endif
    
    /* Use result to prevent optimization */
    sink = checksum;
    
    printf("Test checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
