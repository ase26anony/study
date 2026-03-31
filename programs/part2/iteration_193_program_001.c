/* test_optabs_high_operand.c
 * 
 * This program is designed to trigger GCC's RTL expansion for operations
 * requiring exactly 10 or 11 operands, covering the uncovered switch cases
 * in optabs.cc (lines 8254-8263).
 *
 * Compile with: gcc -O3 -march=native -fno-tree-vectorize -fprofile-arcs -ftest-coverage -o test_optabs test_optabs_high_operand.c
 * Run with: ./test_optabs
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization and ensure expansion occurs in the compiler */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* SSE intrinsics if available */
#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __SSE2__
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

/* Pattern A: Vector blend/select with many conditions (x86 SSE/AVX) */
NOINLINE
v4sf pattern_a_blend_many_operands(v4sf a, v4sf b, v4sf c, v4sf d,
                                   v4sf e, v4sf f, v4sf g, v4sf h,
                                   int mask1, int mask2, int mask3) {
    /* Complex blend operation that may expand to many operands */
    v4sf t1 = a + b;
    v4sf t2 = c * d;
    v4sf t3 = e - f;
    v4sf t4 = g / h;
    
    /* Use conditional selects based on masks - each may become separate operand */
    v4sf r1 = (mask1 > 0) ? t1 : t2;
    v4sf r2 = (mask2 > 0) ? t3 : t4;
    v4sf r3 = (mask3 > 0) ? r1 : r2;
    
    /* Additional operations to increase operand count */
    r3 = r3 + a * b + c * d - e * f;
    
    return r3;
}

/* Pattern B: Fused multiply-add chain (generic, may use fma builtins) */
NOINLINE
float pattern_b_fma_chain(float a, float b, float c, float d,
                          float e, float f, float g, float h,
                          float i, float j, float k) {
    /* Deep FMA chain that may be flattened into many operands */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(t1, t2, t3);
    float t5 = __builtin_fma(t4, j, k);
    
    /* Additional arithmetic to ensure expansion */
    t5 = t5 + a * b - c * d + e * f - g * h + i * j;
    
    return t5;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE
float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually extract and sum all elements - creates many extract operations */
    float sum = 0.0f;
    
    /* Extract each element (each extract may be an operand) */
    sum += v1[0] + v1[1] + v1[2] + v1[3];
    sum += v2[0] + v2[1] + v2[2] + v2[3];
    sum += v3[0] + v3[1] + v3[2] + v3[3];
    sum += v4[0] + v4[1] + v4[2] + v4[3];
    
    /* Additional operations to increase complexity */
    sum = sum * v1[0] - v2[1] + v3[2] / v4[3];
    
    return sum;
}

/* Pattern D: Conditional vector move/merge using SSE intrinsics */
#ifdef __SSE__
NOINLINE
__m128 pattern_d_conditional_blend(__m128 a, __m128 b, __m128 c,
                                   __m128 d, __m128 e, __m128 f,
                                   int imm1, int imm2, int imm3, int imm4) {
    /* Complex sequence of comparisons and blends */
    __m128 cmp1 = _mm_cmplt_ps(a, b);
    __m128 cmp2 = _mm_cmpgt_ps(c, d);
    __m128 cmp3 = _mm_cmpeq_ps(e, f);
    
    /* Blend operations - each may add multiple operands */
    __m128 t1 = _mm_blendv_ps(a, b, cmp1);
    __m128 t2 = _mm_blendv_ps(c, d, cmp2);
    __m128 t3 = _mm_blendv_ps(e, f, cmp3);
    
    /* Shuffle with immediate - adds immediate operand */
    __m128 s1 = _mm_shuffle_ps(t1, t2, imm1);
    __m128 s2 = _mm_shuffle_ps(t2, t3, imm2);
    __m128 s3 = _mm_shuffle_ps(t3, t1, imm3);
    
    /* Final blend */
    __m128 result = _mm_blend_ps(s1, s2, imm4);
    result = _mm_add_ps(result, s3);
    
    return result;
}
#endif

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE
int pattern_e_asm_many_operands(int a, int b, int c, int d, int e,
                                int f, int g, int h, int i, int j, int k) {
    int result1, result2;
    
    /* Inline asm with 11 explicit operands (10 inputs, 1 output) */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "mov %[k], %[out1]\n\t"
        "imul %[out1], %[out2]"
        : [out1] "=r" (result1), [out2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Vector shuffle with many operands (AVX) */
#ifdef __AVX__
NOINLINE
__m256 pattern_f_avx_shuffle(__m256 a, __m256 b, __m256 c, __m256 d,
                             int imm1, int imm2, int imm3, int imm4,
                             int imm5, int imm6) {
    /* AVX shuffle operations with multiple immediates */
    __m256 t1 = _mm256_shuffle_ps(a, b, imm1);
    __m256 t2 = _mm256_shuffle_ps(c, d, imm2);
    __m256 t3 = _mm256_permute2f128_ps(t1, t2, imm3);
    __m256 t4 = _mm256_blend_ps(t1, t2, imm4);
    __m256 t5 = _mm256_blend_ps(t3, t4, imm5);
    
    /* Additional operation with immediate */
    __m256 result = _mm256_permute_ps(t5, imm6);
    
    return result;
}
#endif

/* Main test driver */
int main(int argc, char **argv) {
    volatile float checksum = 0.0f;
    
    /* Initialize test data with some variability based on argc */
    v4sf va = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vd = {13.0f, 14.0f, 15.0f, 16.0f};
    
    int mask1 = argc > 1 ? 1 : 0;
    int mask2 = argc > 2 ? 1 : 0;
    int mask3 = argc > 3 ? 1 : 0;
    
    /* Execute Pattern A */
    v4sf ra = pattern_a_blend_many_operands(va, vb, vc, vd, va, vb, vc, vd,
                                           mask1, mask2, mask3);
    checksum += ra[0] + ra[1] + ra[2] + ra[3];
    
    /* Execute Pattern B */
    float rb = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
                                   7.7f, 8.8f, 9.9f, 10.1f, 11.11f);
    checksum += rb;
    
    /* Execute Pattern C */
    float rc = pattern_c_vector_reduction(va, vb, vc, vd);
    checksum += rc;
    
#ifdef __SSE__
    /* Execute Pattern D */
    __m128 sa = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sb = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 sc = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
    __m128 sd = _mm_set_ps(13.0f, 14.0f, 15.0f, 16.0f);
    __m128 se = _mm_set_ps(17.0f, 18.0f, 19.0f, 20.0f);
    __m128 sf = _mm_set_ps(21.0f, 22.0f, 23.0f, 24.0f);
    
    __m128 rd = pattern_d_conditional_blend(sa, sb, sc, sd, se, sf,
                                           mask1, mask2, mask3, mask1);
    float rd_arr[4];
    _mm_store_ps(rd_arr, rd);
    checksum += rd_arr[0] + rd_arr[1] + rd_arr[2] + rd_arr[3];
#endif
    
    /* Execute Pattern E */
    int re = pattern_e_asm_many_operands(argc, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    checksum += re;
    
#ifdef __AVX__
    /* Execute Pattern F */
    __m256 avxa = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 avxb = _mm256_set_ps(9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
    __m256 avxc = _mm256_set_ps(17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f);
    __m256 avxd = _mm256_set_ps(25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f);
    
    __m256 rf = pattern_f_avx_shuffle(avxa, avxb, avxc, avxd,
                                     mask1, mask2, mask3, mask1, mask2, mask3);
    float rf_arr[8];
    _mm256_store_ps(rf_arr, rf);
    for (int i = 0; i < 8; i++) checksum += rf_arr[i];
#endif
    
    /* Use checksum to prevent optimization */
    printf("Checksum: %f\n", checksum);
    
    return (checksum > 100.0f) ? 0 : 1;
}
