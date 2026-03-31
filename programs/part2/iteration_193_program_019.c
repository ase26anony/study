/* test_optabs_high_operand_count.c
 * 
 * This test aims to cover the 10 and 11 operand switch cases in optabs.cc
 * by generating complex RTL patterns with many operands during expansion.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Define vector types for portability */
#if defined(__SSE__) || defined(__x86_64__) || defined(__i386__)
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#define USE_X86_INTRINSICS 1
#endif

#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>
typedef float32x4_t v4sf;
typedef int32x4_t v4si;
#define USE_ARM_INTRINSICS 1
#endif

/* Fallback dummy types if no SIMD support */
#ifndef USE_X86_INTRINSICS
#ifndef USE_ARM_INTRINSICS
typedef struct { float f[4]; } v4sf;
typedef struct { int i[4]; } v4si;
#endif
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa))
#define USED __attribute__((used))

/* Pattern A: Complex vector blend with many operands */
NOINLINE USED v4sf pattern_a_blend_many_operands(v4sf a, v4sf b, v4sf c, v4sf d,
                                                  v4sf e, v4sf f, v4sf g, v4sf h,
                                                  int mask1, int mask2, int mask3) {
    v4sf result;
    
#if defined(USE_X86_INTRINSICS) && defined(__SSE4_1__)
    /* __builtin_ia32_blendps needs 3 operands but expansion may create more */
    v4sf temp1 = __builtin_ia32_blendps(a, b, mask1);
    v4sf temp2 = __builtin_ia32_blendps(c, d, mask2);
    v4sf temp3 = __builtin_ia32_blendps(e, f, mask3);
    
    /* Create complex expression tree that may flatten to many operands */
    result = __builtin_ia32_blendps(temp1, temp2, mask1 | mask2);
    result = __builtin_ia32_blendps(result, temp3, mask2 | mask3);
    result = __builtin_ia32_blendps(result, g, mask1);
    result = __builtin_ia32_blendps(result, h, mask2);
    
#elif defined(USE_ARM_INTRINSICS)
    /* Use ARM NEON blend operations */
    uint32x4_t mask_vec1 = vdupq_n_u32(mask1);
    uint32x4_t mask_vec2 = vdupq_n_u32(mask2);
    uint32x4_t mask_vec3 = vdupq_n_u32(mask3);
    
    v4sf temp1 = vbslq_f32(mask_vec1, a, b);
    v4sf temp2 = vbslq_f32(mask_vec2, c, d);
    v4sf temp3 = vbslq_f32(mask_vec3, e, f);
    
    v4sf temp4 = vbslq_f32(mask_vec1 | mask_vec2, temp1, temp2);
    result = vbslq_f32(mask_vec2 | mask_vec3, temp4, temp3);
    result = vbslq_f32(mask_vec1, result, g);
    result = vbslq_f32(mask_vec2, result, h);
    
#else
    /* Fallback scalar implementation */
    result = a; /* Dummy implementation */
#endif
    
    return result;
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE USED float pattern_b_fma_chain(float a, float b, float c, float d,
                                        float e, float f, float g, float h,
                                        float i, float j, float k) {
    float result;
    
#if defined(__FMA__) || defined(__FMA4__)
    /* Chain of FMA operations - may expand to many operands */
    result = __builtin_fma(a, b, c);
    result = __builtin_fma(result, d, e);
    result = __builtin_fma(f, g, result);
    result = __builtin_fma(h, i, result);
    result = __builtin_fma(j, k, result);
    
    /* Add more operations to increase operand count */
    result = __builtin_fma(result, a, b);
    result = __builtin_fma(c, d, result);
    result = __builtin_fma(e, f, result);
    result = __builtin_fma(g, h, result);
    
#else
    /* Manual FMA simulation */
    result = a * b + c;
    result = result * d + e;
    result = f * g + result;
    result = h * i + result;
    result = j * k + result;
#endif
    
    return result;
}

/* Pattern C: Vector reduction with explicit scalarization (many extracts) */
NOINLINE USED float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    float sum = 0.0f;
    
#if defined(USE_X86_INTRINSICS)
    /* Extract each element individually - creates many extract operations */
    sum += __builtin_ia32_vec_ext_v4sf(v1, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v1, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v1, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v1, 3);
    
    sum += __builtin_ia32_vec_ext_v4sf(v2, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v2, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v2, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v2, 3);
    
    sum += __builtin_ia32_vec_ext_v4sf(v3, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v3, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v3, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v3, 3);
    
    sum += __builtin_ia32_vec_ext_v4sf(v4, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v4, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v4, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v4, 3);
    
#elif defined(USE_ARM_INTRINSICS)
    /* ARM NEON element extraction */
    sum += vgetq_lane_f32(v1, 0);
    sum += vgetq_lane_f32(v1, 1);
    sum += vgetq_lane_f32(v1, 2);
    sum += vgetq_lane_f32(v1, 3);
    
    sum += vgetq_lane_f32(v2, 0);
    sum += vgetq_lane_f32(v2, 1);
    sum += vgetq_lane_f32(v2, 2);
    sum += vgetq_lane_f32(v2, 3);
    
    sum += vgetq_lane_f32(v3, 0);
    sum += vgetq_lane_f32(v3, 1);
    sum += vgetq_lane_f32(v3, 2);
    sum += vgetq_lane_f32(v3, 3);
    
    sum += vgetq_lane_f32(v4, 0);
    sum += vgetq_lane_f32(v4, 1);
    sum += vgetq_lane_f32(v4, 2);
    sum += vgetq_lane_f32(v4, 3);
    
#else
    /* Scalar fallback */
    sum = 1.0f; /* Dummy value */
#endif
    
    return sum;
}

/* Pattern D: Inline assembly with exactly 11 operands */
NOINLINE USED int pattern_d_asm_many_operands(int a, int b, int c, int d, int e,
                                              int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline asm with 10 input/output/clobber operands total */
    asm volatile (
        "add %[a], %[b], %[c]\n\t"
        "add %[d], %[e], %[f]\n\t"
        "mul %[result], %[g], %[h]\n\t"
        "add %[result], %[result], %[i]\n\t"
        "add %[result], %[result], %[j]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result;
}

/* Pattern E: Complex shuffle with immediate calculation */
NOINLINE USED v4sf pattern_e_complex_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                                             int imm1, int imm2, int imm3, int imm4) {
    v4sf result;
    
#if defined(USE_X86_INTRINSICS) && defined(__SSE__)
    /* Multiple shuffle operations - each takes 3 operands */
    v4sf temp1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf temp2 = __builtin_ia32_shufps(c, d, imm2);
    
    /* Combine with arithmetic to prevent elimination */
    result = __builtin_ia32_shufps(temp1, temp2, imm3);
    result = __builtin_ia32_shufps(result, a, imm4);
    
    /* Additional operations to increase complexity */
    result = __builtin_ia32_addps(result, temp1);
    result = __builtin_ia32_mulps(result, temp2);
    
#elif defined(USE_ARM_INTRINSICS)
    /* ARM equivalent using multiple operations */
    float32x4_t temp1 = vrev64q_f32(a);
    float32x4_t temp2 = vrev64q_f32(b);
    float32x4_t temp3 = vrev64q_f32(c);
    float32x4_t temp4 = vrev64q_f32(d);
    
    result = vaddq_f32(temp1, temp2);
    result = vaddq_f32(result, temp3);
    result = vaddq_f32(result, temp4);
    
#else
    result = a; /* Dummy */
#endif
    
    return result;
}

/* Main test driver with runtime variability */
int main(int argc, char *argv[]) {
    volatile int checksum = 0;
    
    /* Initialize test data with some variability */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Use argc to select different patterns at runtime
     * (compile-time all paths are present for coverage) */
    switch (argc % 5) {
        case 0:
            checksum += (int)pattern_a_blend_many_operands(vec1, vec2, vec3, vec4,
                                                          vec1, vec2, vec3, vec4,
                                                          0x0F, 0xF0, 0x55).f[0];
            break;
        case 1:
            checksum += (int)pattern_b_fma_chain(1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                                                6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f);
            break;
        case 2:
            checksum += (int)pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
            break;
        case 3:
            checksum += pattern_d_asm_many_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
        case 4:
            checksum += (int)pattern_e_complex_shuffle(vec1, vec2, vec3, vec4,
                                                       0x1B, 0x4E, 0x93, 0x39).f[0];
            break;
    }
    
    /* Use checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
