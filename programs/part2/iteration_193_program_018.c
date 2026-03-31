/* test_optabs_high_operand_count.c
 * 
 * This program is designed to trigger GCC's RTL expansion code paths
 * that handle operations with exactly 10 or 11 operands, specifically
 * targeting uncovered switch cases in optabs.cc lines 8254-8263.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization from eliminating our test patterns */
static volatile int sink;

/* Architecture detection and fallbacks */
#ifdef __SSE__
#define USE_X86_INTRINSICS 1
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#elif defined(__ARM_NEON) || defined(__aarch64__)
#define USE_ARM_INTRINSICS 1
#include <arm_neon.h>
#else
#define USE_GENERIC 1
#endif

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Prevent inlining to ensure expansion happens in this function */
__attribute__((noinline, noipa, used))
static void pattern_a_vector_blend_complex(v4sf *result, v4sf a, v4sf b, 
                                          v4sf c, v4sf d, v4sf e, v4sf f,
                                          int mask0, int mask1, int mask2, int mask3) {
    /* Complex blend pattern that may expand to many operands */
    v4sf temp1 = a + b;
    v4sf temp2 = c * d;
    v4sf temp3 = e - f;
    
    /* Create blend mask from multiple inputs */
    v4si blend_mask = {mask0, mask1, mask2, mask3};
    
    /* Manual blend operation - each lane selection creates multiple operands */
    result[0] = (mask0 > 0) ? temp1 : temp2;
    result[1] = (mask1 > 0) ? temp2 : temp3;
    result[2] = (mask2 > 0) ? temp3 : temp1;
    result[3] = (mask3 > 0) ? temp1 + temp2 : temp2 - temp3;
    
    /* Additional operations to increase operand count */
    result[0] = result[0] * result[1] + result[2] - result[3];
}

#ifdef USE_X86_INTRINSICS
__attribute__((noinline, noipa, used))
static __m128 pattern_b_fma_chain(__m128 a, __m128 b, __m128 c, 
                                  __m128 d, __m128 e, __m128 f,
                                  __m128 g, __m128 h, __m128 i) {
    /* Deep FMA chain that may flatten to many operands */
    __m128 t1 = _mm_fmadd_ps(a, b, c);
    __m128 t2 = _mm_fmadd_ps(d, e, f);
    __m128 t3 = _mm_fmadd_ps(g, h, i);
    
    /* Nested FMAs create complex expression trees */
    __m128 t4 = _mm_fmadd_ps(t1, t2, t3);
    __m128 t5 = _mm_fmsub_ps(t2, t3, t1);
    __m128 t6 = _mm_fnmadd_ps(t3, t1, t2);
    
    /* Final combination with many operands */
    return _mm_add_ps(_mm_add_ps(t4, t5), 
                     _mm_add_ps(t6, _mm_mul_ps(t1, t2)));
}

__attribute__((noinline, noipa, used))
static __m128i pattern_c_shuffle_complex(__m128i a, __m128i b, __m128i c,
                                         __m128i d, int imm0, int imm1,
                                         int imm2, int imm3, int imm4,
                                         int imm5, int imm6) {
    /* Complex shuffle pattern with many immediate operands */
    __m128i t1 = _mm_shuffle_epi32(a, _MM_SHUFFLE(imm0 & 3, imm1 & 3, imm2 & 3, imm3 & 3));
    __m128i t2 = _mm_shuffle_epi32(b, _MM_SHUFFLE(imm4 & 3, imm5 & 3, imm6 & 3, (imm0 + imm1) & 3));
    __m128i t3 = _mm_shuffle_epi32(c, _MM_SHUFFLE((imm1 + imm2) & 3, (imm2 + imm3) & 3, 
                                                  (imm3 + imm4) & 3, (imm4 + imm5) & 3));
    __m128i t4 = _mm_shuffle_epi32(d, _MM_SHUFFLE((imm5 + imm6) & 3, (imm0 + imm6) & 3,
                                                  (imm1 + imm5) & 3, (imm2 + imm4) & 3));
    
    /* Combine with many operations */
    __m128i r1 = _mm_add_epi32(t1, t2);
    __m128i r2 = _mm_sub_epi32(t3, t4);
    __m128i r3 = _mm_mullo_epi32(r1, r2);
    __m128i r4 = _mm_slli_epi32(r3, (imm0 + imm1 + imm2) & 7);
    
    return _mm_add_epi32(_mm_add_epi32(r1, r2), 
                        _mm_add_epi32(r3, r4));
}
#endif

__attribute__((noinline, noipa, used))
static v8sf pattern_d_vector_reduction_unrolled(v8sf v0, v8sf v1, v8sf v2,
                                                v8sf v3, v8sf v4, v8sf v5) {
    /* Manually unrolled vector reduction with many lane extractions */
    float sum = 0.0f;
    
    /* Extract and sum each lane - each extract creates operands */
    sum += v0[0] + v0[1] + v0[2] + v0[3] + v0[4] + v0[5] + v0[6] + v0[7];
    sum += v1[0] + v1[1] + v1[2] + v1[3] + v1[4] + v1[5] + v1[6] + v1[7];
    sum += v2[0] + v2[1] + v2[2] + v2[3] + v2[4] + v2[5] + v2[6] + v2[7];
    sum += v3[0] + v3[1] + v3[2] + v3[3] + v3[4] + v3[5] + v3[6] + v3[7];
    sum += v4[0] + v4[1] + v4[2] + v4[3] + v4[4] + v4[5] + v4[6] + v4[7];
    sum += v5[0] + v5[1] + v5[2] + v5[3] + v5[4] + v5[5] + v5[6] + v5[7];
    
    /* Broadcast result to all lanes */
    v8sf result = {sum, sum, sum, sum, sum, sum, sum, sum};
    return result;
}

__attribute__((noinline, noipa, used))
static void pattern_e_inline_asm_multi_operand(int *out, int a, int b, int c,
                                               int d, int e, int f, int g,
                                               int h, int i, int j, int k) {
    /* Inline assembly with exactly 11 operands */
    asm volatile (
        "add %[a], %[b], %[c]\n\t"
        "mul %[d], %[e], %[f]\n\t"
        "sub %[g], %[h], %[i]\n\t"
        "and %[j], %[k], %[a]\n\t"
        "or  %[out0], %[d], %[g]\n\t"
        "xor %[out1], %[j], %[out0]"
        : [out0] "=r" (out[0]), [out1] "=r" (out[1])
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
}

/* Main test driver with runtime variability */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize test data with some variability */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_d = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf vec_e = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vec_f = {21.0f, 22.0f, 23.0f, 24.0f};
    
    v4sf blend_results[4];
    
    /* Pattern A: Complex vector blend (10+ operands) */
    pattern_a_vector_blend_complex(blend_results, vec_a, vec_b, vec_c,
                                  vec_d, vec_e, vec_f,
                                  argc > 1, argc > 2, argc > 3, argc > 4);
    
    /* Use results to prevent elimination */
    for (int i = 0; i < 4; i++) {
        sink = (int)blend_results[i][0];
        result += sink;
    }
    
#ifdef USE_X86_INTRINSICS
    /* Pattern B: FMA chain */
    __m128 xmm_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 xmm_b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 xmm_c = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
    __m128 xmm_d = _mm_set_ps(13.0f, 14.0f, 15.0f, 16.0f);
    __m128 xmm_e = _mm_set_ps(17.0f, 18.0f, 19.0f, 20.0f);
    __m128 xmm_f = _mm_set_ps(21.0f, 22.0f, 23.0f, 24.0f);
    __m128 xmm_g = _mm_set_ps(25.0f, 26.0f, 27.0f, 28.0f);
    __m128 xmm_h = _mm_set_ps(29.0f, 30.0f, 31.0f, 32.0f);
    __m128 xmm_i = _mm_set_ps(33.0f, 34.0f, 35.0f, 36.0f);
    
    __m128 fma_result = pattern_b_fma_chain(xmm_a, xmm_b, xmm_c, xmm_d, xmm_e,
                                           xmm_f, xmm_g, xmm_h, xmm_i);
    
    float fma_arr[4];
    _mm_store_ps(fma_arr, fma_result);
    result += (int)fma_arr[0] + (int)fma_arr[1] + (int)fma_arr[2] + (int)fma_arr[3];
    
    /* Pattern C: Complex shuffle with many immediates */
    __m128i epi_a = _mm_set_epi32(1, 2, 3, 4);
    __m128i epi_b = _mm_set_epi32(5, 6, 7, 8);
    __m128i epi_c = _mm_set_epi32(9, 10, 11, 12);
    __m128i epi_d = _mm_set_epi32(13, 14, 15, 16);
    
    __m128i shuffle_result = pattern_c_shuffle_complex(epi_a, epi_b, epi_c, epi_d,
                                                       argc, argc+1, argc+2, argc+3,
                                                       argc+4, argc+5, argc+6);
    
    int shuffle_arr[4];
    _mm_store_si128((__m128i*)shuffle_arr, shuffle_result);
    result += shuffle_arr[0] + shuffle_arr[1] + shuffle_arr[2] + shuffle_arr[3];
#endif
    
    /* Pattern D: Unrolled vector reduction */
    v8sf v8_0 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf v8_1 = {9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    v8sf v8_2 = {17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f};
    v8sf v8_3 = {25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f};
    v8sf v8_4 = {33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f, 40.0f};
    v8sf v8_5 = {41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f, 48.0f};
    
    v8sf reduction_result = pattern_d_vector_reduction_unrolled(v8_0, v8_1, v8_2,
                                                               v8_3, v8_4, v8_5);
    
    for (int i = 0; i < 8; i++) {
        result += (int)reduction_result[i];
    }
    
    /* Pattern E: Inline assembly with 11 operands */
    int asm_out[2];
    pattern_e_inline_asm_multi_operand(asm_out, 
                                       argc, argc+1, argc+2, argc+3, argc+4,
                                       argc+5, argc+6, argc+7, argc+8, argc+9,
                                       argc+10);
    
    result += asm_out[0] + asm_out[1];
    
    printf("Test result: %d\n", result);
    return result != 0 ? 0 : 1;
}
