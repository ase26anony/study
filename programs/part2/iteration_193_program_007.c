/* test_optabs_coverage.c
 * 
 * This test program is designed to trigger the 10 and 11 operand switch cases
 * in GCC's optabs.cc during RTL expansion. It uses various patterns that
 * generate many operands during expansion.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline, noipa))
#define USED __attribute__((used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Volatile sink to prevent optimization */
static volatile float volatile_sink;
static volatile int volatile_sink_int;

/* Pattern A: Vector blend with complex mask generation (many operands) */
NOINLINE USED v4sf pattern_a_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                                          v4sf e, v4sf f, v4sf g, v4sf h) {
    /* Complex mask computation that may expand to many operands */
    v4sf cmp1 = a > b;
    v4sf cmp2 = c < d;
    v4sf cmp3 = e == f;
    v4sf cmp4 = g <= h;
    
    /* Combine comparisons - each operation adds operands */
    v4sf mask1 = cmp1 & cmp2;
    v4sf mask2 = cmp3 | cmp4;
    v4sf final_mask = mask1 ^ mask2;
    
    /* Blend operation with complex mask */
    v4sf result = __builtin_shuffle(a, b, (v4si)final_mask);
    
    /* Use result to prevent elimination */
    volatile_sink = result[0] + result[1] + result[2] + result[3];
    return result;
}

/* Pattern B: Fused multiply-add chain (deep expression tree) */
NOINLINE USED float pattern_b_fma_chain(float a, float b, float c, float d,
                                       float e, float f, float g, float h,
                                       float i, float j, float k, float l) {
    /* Chain of FMAs creates deep expression tree */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    
    /* Combine results with more FMAs */
    float r1 = __builtin_fmaf(t1, t2, t3);
    float r2 = __builtin_fmaf(t4, t1, t2);
    float result = __builtin_fmaf(r1, r2, t4);
    
    volatile_sink = result;
    return result;
}

/* Pattern C: Vector reduction with explicit scalarization (many extracts) */
NOINLINE USED float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually extract and sum all elements - each extract is an operand */
    float sum = 0.0f;
    
    /* Extract from v1 */
    sum += __builtin_shufflevector(v1, v1, 0, 0, 0, 0)[0];
    sum += __builtin_shufflevector(v1, v1, 1, 1, 1, 1)[0];
    sum += __builtin_shufflevector(v1, v1, 2, 2, 2, 2)[0];
    sum += __builtin_shufflevector(v1, v1, 3, 3, 3, 3)[0];
    
    /* Extract from v2 */
    sum += __builtin_shufflevector(v2, v2, 0, 0, 0, 0)[0];
    sum += __builtin_shufflevector(v2, v2, 1, 1, 1, 1)[0];
    sum += __builtin_shufflevector(v2, v2, 2, 2, 2, 2)[0];
    sum += __builtin_shufflevector(v2, v2, 3, 3, 3, 3)[0];
    
    /* Extract from v3 */
    sum += __builtin_shufflevector(v3, v3, 0, 0, 0, 0)[0];
    sum += __builtin_shufflevector(v3, v3, 1, 1, 1, 1)[0];
    sum += __builtin_shufflevector(v3, v3, 2, 2, 2, 2)[0];
    sum += __builtin_shufflevector(v3, v3, 3, 3, 3, 3)[0];
    
    /* Extract from v4 */
    sum += __builtin_shufflevector(v4, v4, 0, 0, 0, 0)[0];
    sum += __builtin_shufflevector(v4, v4, 1, 1, 1, 1)[0];
    sum += __builtin_shufflevector(v4, v4, 2, 2, 2, 2)[0];
    sum += __builtin_shufflevector(v4, v4, 3, 3, 3, 3)[0];
    
    volatile_sink = sum;
    return sum;
}

/* Pattern D: Conditional vector move with multi-input comparison */
NOINLINE USED v4sf pattern_d_conditional_move(v4sf a, v4sf b, v4sf c, v4sf d,
                                             v4sf e, v4sf f, v4sf g, v4sf h) {
    /* Complex comparison chain */
    v4sf cmp1 = a > b;
    v4sf cmp2 = c < d;
    v4sf cmp3 = e != f;
    v4sf cmp4 = g >= h;
    
    /* Combine with logical operations */
    v4sf mask_and = cmp1 & cmp2;
    v4sf mask_or = cmp3 | cmp4;
    v4sf final_mask = mask_and ^ mask_or;
    
    /* Conditional select using mask */
    v4sf result = __builtin_shuffle(a, b, (v4si)final_mask);
    
    /* Additional operations to increase operand count */
    result = result * c + d;
    result = result / e - f;
    
    volatile_sink = result[0];
    return result;
}

/* Pattern E: Inline assembly with many operands (direct 10+ operands) */
NOINLINE USED int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                             int f, int g, int h, int i, int j) {
    int result1, result2, result3;
    
    /* Inline assembly with 11 explicit operands */
    asm volatile (
        /* Complex operation with many inputs/outputs */
        "imul %[a], %[b]\n\t"
        "add %[c], %[b]\n\t"
        "sub %[d], %[b]\n\t"
        "and %[e], %[b]\n\t"
        "or %[f], %[b]\n\t"
        "xor %[g], %[b]\n\t"
        "shl $3, %[b]\n\t"
        "shr $1, %[b]\n\t"
        "mov %[b], %[r1]\n\t"
        "add %[h], %[r1]\n\t"
        "imul %[i], %[r1]\n\t"
        "add %[j], %[r1]"
        : [r1] "=r" (result1), [r2] "=r" (result2), [r3] "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc", "memory"
    );
    
    volatile_sink_int = result1 + result2 + result3;
    return result1;
}

/* Pattern F: Vector shuffle with immediate computation (x86-specific) */
#ifdef __SSE__
#include <xmmintrin.h>
NOINLINE USED __m128 pattern_f_shuffle_complex(__m128 a, __m128 b, __m128 c,
                                              __m128 d, int imm1, int imm2,
                                              int imm3, int imm4) {
    /* Multiple shuffle operations with computed immediates */
    __m128 s1 = _mm_shuffle_ps(a, b, imm1);
    __m128 s2 = _mm_shuffle_ps(c, d, imm2);
    __m128 s3 = _mm_shuffle_ps(s1, s2, imm3);
    __m128 s4 = _mm_shuffle_ps(s3, a, imm4);
    
    /* Blend operations */
    __m128 blend1 = _mm_blend_ps(s1, s2, 0x5);
    __m128 blend2 = _mm_blend_ps(s3, s4, 0xA);
    __m128 result = _mm_blend_ps(blend1, blend2, 0xC);
    
    /* Use result */
    float res_arr[4];
    _mm_storeu_ps(res_arr, result);
    volatile_sink = res_arr[0] + res_arr[1] + res_arr[2] + res_arr[3];
    
    return result;
}
#endif

/* Pattern G: ARM NEON multi-operand pattern */
#ifdef __ARM_NEON
#include <arm_neon.h>
NOINLINE USED float32x4_t pattern_g_neon_complex(float32x4_t a, float32x4_t b,
                                                float32x4_t c, float32x4_t d,
                                                float32x4_t e, float32x4_t f) {
    /* Multiple NEON operations */
    float32x4_t add1 = vaddq_f32(a, b);
    float32x4_t mul1 = vmulq_f32(c, d);
    float32x4_t sub1 = vsubq_f32(e, f);
    
    /* Fused multiply-add */
    float32x4_t fma1 = vfmaq_f32(add1, mul1, sub1);
    
    /* Lane operations */
    float32x4_t lane0 = vdupq_n_f32(vgetq_lane_f32(fma1, 0));
    float32x4_t lane1 = vdupq_n_f32(vgetq_lane_f32(fma1, 1));
    float32x4_t lane2 = vdupq_n_f32(vgetq_lane_f32(fma1, 2));
    float32x4_t lane3 = vdupq_n_f32(vgetq_lane_f32(fma1, 3));
    
    /* Combine lanes */
    float32x4_t result = vaddq_f32(vaddq_f32(lane0, lane1),
                                  vaddq_f32(lane2, lane3));
    
    /* Store to volatile */
    float res_arr[4];
    vst1q_f32(res_arr, result);
    volatile_sink = res_arr[0] + res_arr[1] + res_arr[2] + res_arr[3];
    
    return result;
}
#endif

/* Main test driver that exercises all patterns */
int main(int argc, char *argv[]) {
    float result = 0.0f;
    
    /* Initialize test data with some variability */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Use argc to add runtime variability */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Exercise Pattern A */
    v4sf res_a = pattern_a_blend_complex(v1, v2, v3, v4, v2, v3, v4, v1);
    result += res_a[0];
    
    /* Exercise Pattern B */
    float res_b = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
                                      7.7f, 8.8f, 9.9f, 10.1f, 11.1f, 12.1f);
    result += res_b;
    
    /* Exercise Pattern C */
    float res_c = pattern_c_vector_reduction(v1, v2, v3, v4);
    result += res_c;
    
    /* Exercise Pattern D */
    v4sf res_d = pattern_d_conditional_move(v1, v2, v3, v4, v2, v3, v4, v1);
    result += res_d[0];
    
    /* Exercise Pattern E */
    int res_e = pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += (float)res_e;
    
    /* Exercise architecture-specific patterns based on availability */
#ifdef __SSE__
    __m128 sse_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 sse_c = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
    __m128 sse_d = _mm_set_ps(13.0f, 14.0f, 15.0f, 16.0f);
    
    __m128 res_f = pattern_f_shuffle_complex(sse_a, sse_b, sse_c, sse_d,
                                            0x1B, 0x27, 0x39, 0x4E);
    float f_arr[4];
    _mm_storeu_ps(f_arr, res_f);
    result += f_arr[0];
#endif
    
#ifdef __ARM_NEON
    float32x4_t neon_a = {1.0f, 2.0f, 3.0f, 4.0f};
    float32x4_t neon_b = {5.0f, 6.0f, 7.0f, 8.0f};
    float32x4_t neon_c = {9.0f, 10.0f, 11.0f, 12.0f};
    float32x4_t neon_d = {13.0f, 14.0f, 15.0f, 16.0f};
    float32x4_t neon_e = {17.0f, 18.0f, 19.0f, 20.0f};
    float32x4_t neon_f = {21.0f, 22.0f, 23.0f, 24.0f};
    
    float32x4_t res_g = pattern_g_neon_complex(neon_a, neon_b, neon_c,
                                              neon_d, neon_e, neon_f);
    float g_arr[4];
    vst1q_f32(g_arr, res_g);
    result += g_arr[0];
#endif
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %f\n", result);
    
    return (result > 0.0f) ? 0 : 1;
}
