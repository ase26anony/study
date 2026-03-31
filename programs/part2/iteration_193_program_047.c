/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations that might eliminate our test patterns */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Architecture-specific vector types */
#ifdef __SSE__
#include <xmmintrin.h>
typedef __m128 v4sf;
typedef __m128i v4si;
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

#ifdef __AVX__
#include <immintrin.h>
typedef __m256 v8sf;
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
typedef float32x4_t v4sf_neon;
#endif

/* Volatile variable to prevent dead code elimination */
static volatile int g_volatile_result = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE v4sf pattern_a_shuffle_many_operands(v4sf a, v4sf b, v4sf c, v4sf d,
                                              int mask1, int mask2, int mask3, int mask4) {
    /* This should expand to many operands during RTL generation */
    v4sf result;
    
#ifdef __SSE__
    /* Shuffle a and b with mask1 */
    v4sf ab_shuf = _mm_shuffle_ps(a, b, mask1);
    
    /* Shuffle c and d with mask2 */
    v4sf cd_shuf = _mm_shuffle_ps(c, d, mask2);
    
    /* Blend the two results with mask3 */
    result = _mm_blend_ps(ab_shuf, cd_shuf, mask3);
    
    /* Final shuffle with mask4 */
    result = _mm_shuffle_ps(result, result, mask4);
#else
    /* Fallback implementation */
    float* a_f = (float*)&a;
    float* b_f = (float*)&b;
    float* c_f = (float*)&c;
    float* d_f = (float*)&d;
    float* r_f = (float*)&result;
    
    /* Complex manual shuffle pattern */
    r_f[0] = (mask1 & 0x01) ? a_f[0] : b_f[0];
    r_f[1] = (mask1 & 0x02) ? a_f[1] : b_f[1];
    r_f[2] = (mask2 & 0x04) ? c_f[2] : d_f[2];
    r_f[3] = (mask2 & 0x08) ? c_f[3] : d_f[3];
    
    /* Blend operations */
    if (mask3 & 0x01) r_f[0] = cd_shuf[0];
    if (mask3 & 0x02) r_f[1] = cd_shuf[1];
#endif
    
    return result;
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE float pattern_b_fma_chain(float a, float b, float c, float d, 
                                   float e, float f, float g, float h) {
    /* Chain of operations that may expand to many operands */
    float result;
    
#ifdef __FMA__
    /* Use builtin_fma if available - creates complex expression trees */
    result = __builtin_fmaf(a, b, 
              __builtin_fmaf(c, d,
                __builtin_fmaf(e, f,
                  __builtin_fmaf(g, h, 0.0f))));
#else
    /* Manual FMA simulation */
    result = a * b + c * d + e * f + g * h;
#endif
    
    return result;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually unrolled horizontal addition across multiple vectors */
    float sum = 0.0f;
    
#ifdef __SSE__
    /* Extract each element individually - creates many extract operations */
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
    float* v1_f = (float*)&v1;
    float* v2_f = (float*)&v2;
    float* v3_f = (float*)&v3;
    float* v4_f = (float*)&v4;
    
    for (int i = 0; i < 4; i++) {
        sum += v1_f[i] + v2_f[i] + v3_f[i] + v4_f[i];
    }
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE v4sf pattern_d_conditional_vector(v4sf a, v4sf b, v4sf c, v4sf d,
                                           v4sf e, v4sf f) {
    v4sf result;
    
#ifdef __SSE__
    /* Multiple comparisons and blends */
    v4sf cmp1 = _mm_cmplt_ps(a, b);
    v4sf cmp2 = _mm_cmpgt_ps(c, d);
    v4sf cmp3 = _mm_cmpeq_ps(e, f);
    
    /* Combine comparison results */
    v4sf mask1 = _mm_and_ps(cmp1, cmp2);
    v4sf mask2 = _mm_or_ps(mask1, cmp3);
    
    /* Conditional selection based on complex mask */
    result = _mm_blendv_ps(a, b, mask2);
    result = _mm_blendv_ps(result, c, cmp1);
    result = _mm_blendv_ps(result, d, cmp2);
#else
    result = a; /* Fallback */
#endif
    
    return result;
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE int pattern_e_many_operand_asm(int a, int b, int c, int d, int e,
                                        int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline assembly with 10 input/output operands plus clobber */
    asm volatile (
        /* Complex operation using all inputs */
        "addl %[a], %[b]\n\t"
        "addl %[c], %[d]\n\t"
        "addl %[e], %[f]\n\t"
        "addl %[g], %[h]\n\t"
        "imull %[i], %[j]\n\t"
        "addl %%eax, %[result]\n\t"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "eax", "cc"
    );
    
    return result;
}

/* Pattern F: ARM NEON specific - many operand vector operations */
#ifdef __ARM_NEON
NOINLINE v4sf_neon pattern_f_neon_many_ops(v4sf_neon a, v4sf_neon b, 
                                           v4sf_neon c, v4sf_neon d) {
    /* Complex NEON operations that may expand to many RTL operands */
    v4sf_neon result;
    
    /* Multiple lane operations */
    result = vaddq_f32(a, b);
    result = vmlaq_f32(result, c, d);  /* Fused multiply-add */
    
    /* Lane extractions and inserts */
    float32_t lane0 = vgetq_lane_f32(result, 0);
    float32_t lane1 = vgetq_lane_f32(result, 1);
    float32_t lane2 = vgetq_lane_f32(result, 2);
    float32_t lane3 = vgetq_lane_f32(result, 3);
    
    /* Reconstruct with different ordering */
    result = vsetq_lane_f32(lane3, result, 0);
    result = vsetq_lane_f32(lane2, result, 1);
    result = vsetq_lane_f32(lane1, result, 2);
    result = vsetq_lane_f32(lane0, result, 3);
    
    return result;
}
#endif

/* Main test driver */
int main(int argc, char** argv) {
    int checksum = 0;
    
    /* Initialize test data with some variability based on argc */
    float f1 = (argc > 1) ? atof(argv[1]) : 1.0f;
    float f2 = (argc > 2) ? atof(argv[2]) : 2.0f;
    float f3 = (argc > 3) ? atof(argv[3]) : 3.0f;
    float f4 = (argc > 4) ? atof(argv[4]) : 4.0f;
    
    /* Create vector values */
    v4sf v1 = {f1, f2, f3, f4};
    v4sf v2 = {f2, f3, f4, f1};
    v4sf v3 = {f3, f4, f1, f2};
    v4sf v4 = {f4, f1, f2, f3};
    
    /* Execute all patterns to ensure they're compiled */
    
    /* Pattern A */
    v4sf res_a = pattern_a_shuffle_many_operands(v1, v2, v3, v4, 
                                                0x1B, 0x27, 0x9, 0xC6);
    checksum += ((int*)&res_a)[0];
    
    /* Pattern B */
    float res_b = pattern_b_fma_chain(f1, f2, f3, f4, f1, f2, f3, f4);
    checksum += (int)res_b;
    
    /* Pattern C */
    float res_c = pattern_c_vector_reduction(v1, v2, v3, v4);
    checksum += (int)res_c;
    
    /* Pattern D */
    v4sf res_d = pattern_d_conditional_vector(v1, v2, v3, v4, v1, v2);
    checksum += ((int*)&res_d)[0];
    
    /* Pattern E */
    int res_e = pattern_e_many_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    checksum += res_e;
    
#ifdef __ARM_NEON
    /* Pattern F for ARM */
    v4sf_neon neon_a = {f1, f2, f3, f4};
    v4sf_neon neon_b = {f2, f3, f4, f1};
    v4sf_neon neon_c = {f3, f4, f1, f2};
    v4sf_neon neon_d = {f4, f1, f2, f3};
    v4sf_neon res_f = pattern_f_neon_many_ops(neon_a, neon_b, neon_c, neon_d);
    checksum += (int)vgetq_lane_f32(res_f, 0);
#endif
    
    /* Store to volatile to prevent optimization */
    g_volatile_result = checksum;
    
    printf("Test checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;  /* Non-zero exit if checksum non-zero */
}
