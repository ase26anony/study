/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */
/* Compile with: gcc -O3 -march=native -fno-tree-vectorize -fprofile-arcs -ftest-coverage test_optabs.c -o test_optabs_executable */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for portability */
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#define USE_X86_INTRINSICS 1
#elif defined(__ARM_NEON) || defined(__aarch64__)
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v2sf __attribute__((vector_size(8)));
typedef int v2si __attribute__((vector_size(8)));
#define USE_ARM_INTRINSICS 1
#else
/* Fallback dummy types */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa))
#define USED __attribute__((used))

/* Global volatile to prevent dead code elimination */
static volatile int g_volatile_result = 0;

/* Pattern A: Vector blend with complex mask computation (targets 10+ operands) */
NOINLINE USED v4sf pattern_a_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                                          v4sf e, v4sf f, v4sf g, v4sf h) {
    /* Complex blend operation that may expand to many operands */
    v4sf temp1 = a + b;
    v4sf temp2 = c * d;
    v4sf temp3 = e - f;
    v4sf temp4 = g / h;
    
    /* Create complex condition from multiple operations */
    v4sf cmp1 = temp1 > temp2;
    v4sf cmp2 = temp3 < temp4;
    v4sf cmp3 = (a * b) == (c * d);
    v4sf cmp4 = (e + f) != (g + h);
    
    /* Combine conditions - each operation adds operands */
    v4sf mask = (cmp1 & cmp2) | (cmp3 & cmp4);
    
    /* Final blend-like operation */
    v4sf result = (mask) ? temp1 : temp2;
    result += (cmp1) ? temp3 : temp4;
    
    return result;
}

/* Pattern B: Fused multiply-add chain with many accumulators */
NOINLINE USED float pattern_b_fma_chain(float a, float b, float c, float d,
                                       float e, float f, float g, float h,
                                       float i, float j, float k, float l) {
    /* Deep FMA expression tree that may flatten to many operands */
    float res1 = a * b + c;
    float res2 = d * e + f;
    float res3 = g * h + i;
    float res4 = j * k + l;
    
    /* Chain them together */
    float result = res1 * res2 + res3 * res4;
    result = result * a + b * c;
    result = result * d + e * f;
    result = result * g + h * i;
    
    return result;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE USED float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually extract and sum vector elements - creates many extract operations */
    float sum = 0.0f;
    
    /* Extract each element (each extract may be an operand) */
    sum += v1[0] + v1[1] + v1[2] + v1[3];
    sum += v2[0] + v2[1] + v2[2] + v2[3];
    sum += v3[0] + v3[1] + v3[2] + v3[3];
    sum += v4[0] + v4[1] + v4[2] + v4[3];
    
    /* Additional operations to increase operand count */
    sum = sum * v1[0] - v2[1];
    sum = sum / v3[2] + v4[3];
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE USED v4sf pattern_d_conditional_vector(v4sf a, v4sf b, v4sf c, v4sf d,
                                               v4sf e, v4sf f, v4sf g, v4sf h) {
    /* Multiple vector comparisons combined */
    v4sf cmp_ab = a > b;
    v4sf cmp_cd = c < d;
    v4sf cmp_ef = e == f;
    v4sf cmp_gh = g != h;
    
    /* Combine comparisons with logical operations */
    v4sf mask1 = cmp_ab & cmp_cd;
    v4sf mask2 = cmp_ef | cmp_gh;
    v4sf mask3 = mask1 ^ mask2;
    
    /* Use masks to select values */
    v4sf sel1 = (mask1) ? a : b;
    v4sf sel2 = (mask2) ? c : d;
    v4sf sel3 = (mask3) ? e : f;
    
    /* Final combination */
    v4sf result = sel1 * sel2 + sel3;
    result = result / (a + b) * (c - d);
    
    return result;
}

/* Pattern E: Direct inline assembly with many operands */
NOINLINE USED int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                             int f, int g, int h, int i, int j) {
    int result1, result2, result3;
    
    /* Inline assembly with exactly 11 operands (including clobbers) */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "mul %[r3], %[e], %[f]\n\t"
        "add %[r1], %[r1], %[g]\n\t"
        "sub %[r2], %[r2], %[h]\n\t"
        "and %[r3], %[r3], %[i]\n\t"
        "or  %[r1], %[r1], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2), [r3] "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Pattern F: Vector shuffle with dynamic indices (x86 specific) */
#if USE_X86_INTRINSICS
#include <xmmintrin.h>
#include <emmintrin.h>

NOINLINE USED __m128 pattern_f_shuffle_complex(__m128 a, __m128 b, __m128 c, __m128 d,
                                              int imm1, int imm2, int imm3, int imm4) {
    /* Multiple shuffle operations with different masks */
    __m128 shuf1 = _mm_shuffle_ps(a, b, imm1);
    __m128 shuf2 = _mm_shuffle_ps(c, d, imm2);
    __m128 shuf3 = _mm_shuffle_ps(shuf1, shuf2, imm3);
    __m128 shuf4 = _mm_shuffle_ps(b, c, imm4);
    
    /* Blend operations */
    __m128 blend1 = _mm_blend_ps(shuf1, shuf2, 0x5);
    __m128 blend2 = _mm_blend_ps(shuf3, shuf4, 0xA);
    
    /* Final combination */
    __m128 result = _mm_add_ps(blend1, blend2);
    result = _mm_mul_ps(result, a);
    result = _mm_sub_ps(result, b);
    
    return result;
}
#endif

/* Pattern G: ARM NEON multi-operand operations */
#if USE_ARM_INTRINSICS
#include <arm_neon.h>

NOINLINE USED float32x4_t pattern_g_neon_complex(float32x4_t a, float32x4_t b,
                                                float32x4_t c, float32x4_t d,
                                                float32x4_t e, float32x4_t f) {
    /* Multiple NEON operations chained together */
    float32x4_t vadd1 = vaddq_f32(a, b);
    float32x4_t vadd2 = vaddq_f32(c, d);
    float32x4_t vmul1 = vmulq_f32(e, f);
    float32x4_t vmul2 = vmulq_f32(vadd1, vadd2);
    
    /* Comparisons */
    uint32x4_t vcmp1 = vcgtq_f32(a, b);
    uint32x4_t vcmp2 = vcltq_f32(c, d);
    
    /* Bitwise operations on comparison results */
    uint32x4_t vand = vandq_u32(vcmp1, vcmp2);
    uint32x4_t vorr = vorrq_u32(vcmp1, vcmp2);
    
    /* Select operations using masks */
    float32x4_t vsel1 = vbslq_f32(vand, vadd1, vmul1);
    float32x4_t vsel2 = vbslq_f32(vorr, vadd2, vmul2);
    
    /* Final combination */
    float32x4_t result = vaddq_f32(vsel1, vsel2);
    result = vmulq_f32(result, a);
    
    return result;
}
#endif

/* Main test driver */
int main(int argc, char *argv[]) {
    float final_result = 0.0f;
    
    /* Initialize test vectors with varying values based on argc */
    v4sf v1 = {1.0f + argc, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f + argc, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f + argc, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f + argc};
    v4sf v5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf v6 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf v7 = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf v8 = {29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Execute all patterns to ensure compilation and coverage */
    
    /* Pattern A - Vector blend with complex operations */
    v4sf res_a = pattern_a_blend_complex(v1, v2, v3, v4, v5, v6, v7, v8);
    final_result += res_a[0] + res_a[1] + res_a[2] + res_a[3];
    
    /* Pattern B - FMA chain with many scalar operands */
    float res_b = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
                                      7.7f, 8.8f, 9.9f, 10.1f, 11.1f, 12.1f);
    final_result += res_b;
    
    /* Pattern C - Vector reduction with explicit extraction */
    float res_c = pattern_c_vector_reduction(v1, v2, v3, v4);
    final_result += res_c;
    
    /* Pattern D - Conditional vector operations */
    v4sf res_d = pattern_d_conditional_vector(v1, v2, v3, v4, v5, v6, v7, v8);
    final_result += res_d[0] - res_d[1] + res_d[2] - res_d[3];
    
    /* Pattern E - Multi-operand inline assembly */
    int res_e = pattern_e_multi_operand_asm(argc, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    final_result += (float)res_e;
    
    /* Architecture-specific patterns */
#if USE_X86_INTRINSICS
    /* Pattern F - x86 shuffle with many operands */
    __m128 xmm1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f + argc);
    __m128 xmm2 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    __m128 xmm3 = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
    __m128 xmm4 = _mm_set_ps(13.0f, 14.0f, 15.0f, 16.0f);
    
    __m128 res_f = pattern_f_shuffle_complex(xmm1, xmm2, xmm3, xmm4,
                                           0x1B, 0x27, 0x39, 0x4E);
    float f_res[4];
    _mm_store_ps(f_res, res_f);
    final_result += f_res[0] + f_res[1] + f_res[2] + f_res[3];
#endif
    
#if USE_ARM_INTRINSICS
    /* Pattern G - ARM NEON complex operations */
    float32x4_t neon1 = vld1q_f32((const float32_t*)&v1[0]);
    float32x4_t neon2 = vld1q_f32((const float32_t*)&v2[0]);
    float32x4_t neon3 = vld1q_f32((const float32_t*)&v3[0]);
    float32x4_t neon4 = vld1q_f32((const float32_t*)&v4[0]);
    float32x4_t neon5 = vld1q_f32((const float32_t*)&v5[0]);
    float32x4_t neon6 = vld1q_f32((const float32_t*)&v6[0]);
    
    float32x4_t res_g = pattern_g_neon_complex(neon1, neon2, neon3, neon4, neon5, neon6);
    float g_res[4];
    vst1q_f32(g_res, res_g);
    final_result += g_res[0] + g_res[1] + g_res[2] + g_res[3];
#endif
    
    /* Store to volatile global to prevent optimization */
    g_volatile_result = (int)final_result;
    
    /* Return checksum to ensure execution */
    printf("Result: %f\n", final_result);
    return (final_result > 0) ? 0 : 1;
}
