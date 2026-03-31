/*
 * test_optabs_high_operand_count.c
 * 
 * This program is designed to trigger GCC's RTL expansion for operations
 * requiring exactly 10 or 11 operands, covering the uncovered switch cases
 * in optabs.cc lines 8254-8263.
 *
 * Compilation flags recommended:
 *   x86: gcc -O3 -march=skylake -fno-tree-vectorize -fprofile-arcs -ftest-coverage -fdump-rtl-expand -o test_optabs test_optabs_high_operand_count.c
 *   ARM: gcc -O3 -march=armv8-a+simd -fno-tree-vectorize -fprofile-arcs -ftest-coverage -fdump-rtl-expand -o test_optabs test_optabs_high_operand_count.c
 */

#include <stdint.h>
#include <stdio.h>

/* Prevent optimization of critical functions */
#define NOINLINE_NOIPA __attribute__((noinline, noipa, used))

/* Define vector types for portability */
#if defined(__SSE__) || defined(__AVX__)
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
typedef float32x4_t v4sf;
typedef int32x4_t v4si;
#endif

/* Generic fallback for compilers without SIMD support */
#ifndef __SSE__
#ifndef __ARM_NEON
typedef struct { float f[4]; } v4sf;
typedef struct { int i[4]; } v4si;
#endif
#endif

/* Pattern A: Vector blend with complex mask computation (10+ operands) */
NOINLINE_NOIPA
v4sf pattern_a_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                             v4sf e, v4sf f, v4sf g, v4sf h,
                             int mask0, int mask1, int mask2, int mask3) {
#if defined(__SSE4_1__)
    /* __builtin_ia32_blendps takes 2 vectors + immediate = 3 operands,
       but the expansion may generate many more when mask is variable */
    v4sf t1 = __builtin_ia32_blendps(a, b, mask0);
    v4sf t2 = __builtin_ia32_blendps(c, d, mask1);
    v4sf t3 = __builtin_ia32_blendps(e, f, mask2);
    v4sf t4 = __builtin_ia32_blendps(g, h, mask3);
    
    /* Combine results - creates dependency chain */
    v4sf r1 = t1 + t2;
    v4sf r2 = t3 + t4;
    return __builtin_ia32_blendps(r1, r2, (mask0 ^ mask1) & 0xF);
#elif defined(__ARM_NEON)
    /* Use vbslq_f32 for variable blend */
    uint32x4_t mask_vec = {mask0, mask1, mask2, mask3};
    v4sf t1 = vbslq_f32(vdupq_n_u32(mask0), a, b);
    v4sf t2 = vbslq_f32(vdupq_n_u32(mask1), c, d);
    v4sf t3 = vbslq_f32(vdupq_n_u32(mask2), e, f);
    v4sf t4 = vbslq_f32(vdupq_n_u32(mask3), g, h);
    
    v4sf r1 = vaddq_f32(t1, t2);
    v4sf r2 = vaddq_f32(t3, t4);
    return vbslq_f32(mask_vec, r1, r2);
#else
    /* Fallback scalar implementation */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float val = ((mask0 >> i) & 1) ? a.f[i] : b.f[i];
        val += ((mask1 >> i) & 1) ? c.f[i] : d.f[i];
        val += ((mask2 >> i) & 1) ? e.f[i] : f.f[i];
        val += ((mask3 >> i) & 1) ? g.f[i] : h.f[i];
        result.f[i] = val;
    }
    return result;
#endif
}

/* Pattern B: Fused multiply-add chain (11+ operands in expansion) */
NOINLINE_NOIPA
v4sf pattern_b_fma_chain(v4sf a, v4sf b, v4sf c, v4sf d,
                         v4sf e, v4sf f, v4sf g, v4sf h,
                         v4sf i, v4sf j, v4sf k, v4sf l) {
#if defined(__FMA__) || defined(__AVX2__)
    /* Chain of FMA operations - each FMA expands to 3 inputs + 1 output = 4 operands,
       but when chained they may be flattened into a single multi-operand expression */
    v4sf t1 = __builtin_fma(a, b, c);
    v4sf t2 = __builtin_fma(d, e, f);
    v4sf t3 = __builtin_fma(g, h, i);
    v4sf t4 = __builtin_fma(j, k, l);
    
    /* Combine with more arithmetic */
    v4sf r1 = t1 * t2;
    v4sf r2 = t3 * t4;
    return __builtin_fma(r1, r2, t1 + t2 + t3 + t4);
#elif defined(__ARM_NEON) && defined(__ARM_FEATURE_FMA)
    v4sf t1 = vfmaq_f32(c, a, b);
    v4sf t2 = vfmaq_f32(f, d, e);
    v4sf t3 = vfmaq_f32(i, g, h);
    v4sf t4 = vfmaq_f32(l, j, k);
    
    v4sf r1 = vmulq_f32(t1, t2);
    v4sf r2 = vmulq_f32(t3, t4);
    v4sf sum = vaddq_f32(vaddq_f32(t1, t2), vaddq_f32(t3, t4));
    return vfmaq_f32(sum, r1, r2);
#else
    /* Fallback */
    v4sf result;
    for (int idx = 0; idx < 4; idx++) {
        float t1 = a.f[idx] * b.f[idx] + c.f[idx];
        float t2 = d.f[idx] * e.f[idx] + f.f[idx];
        float t3 = g.f[idx] * h.f[idx] + i.f[idx];
        float t4 = j.f[idx] * k.f[idx] + l.f[idx];
        float r1 = t1 * t2;
        float r2 = t3 * t4;
        result.f[idx] = r1 * r2 + (t1 + t2 + t3 + t4);
    }
    return result;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization (10+ extract operations) */
NOINLINE_NOIPA
float pattern_c_scalarized_reduction(v4sf v0, v4sf v1, v4sf v2, v4sf v3,
                                     v4sf v4, v4sf v5, v4sf v6, v4sf v7) {
    float sum = 0.0f;
    
    /* Manually extract and sum all elements from 8 vectors = 32 extractions */
#if defined(__SSE__)
    sum += __builtin_ia32_vec_ext_v4sf(v0, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v0, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v0, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v0, 3);
    
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
    
    /* Continue with more extracts to ensure many operands */
    sum += __builtin_ia32_vec_ext_v4sf(v4, 0);
    sum += __builtin_ia32_vec_ext_v4sf(v4, 1);
    sum += __builtin_ia32_vec_ext_v4sf(v4, 2);
    sum += __builtin_ia32_vec_ext_v4sf(v4, 3);
#elif defined(__ARM_NEON)
    sum += vgetq_lane_f32(v0, 0);
    sum += vgetq_lane_f32(v0, 1);
    sum += vgetq_lane_f32(v0, 2);
    sum += vgetq_lane_f32(v0, 3);
    
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
    /* Fallback */
    for (int i = 0; i < 4; i++) sum += v0.f[i];
    for (int i = 0; i < 4; i++) sum += v1.f[i];
    for (int i = 0; i < 4; i++) sum += v2.f[i];
    for (int i = 0; i < 4; i++) sum += v3.f[i];
    for (int i = 0; i < 4; i++) sum += v4.f[i];
    for (int i = 0; i < 4; i++) sum += v5.f[i];
    for (int i = 0; i < 4; i++) sum += v6.f[i];
    for (int i = 0; i < 4; i++) sum += v7.f[i];
#endif
    
    return sum;
}

/* Pattern D: Inline assembly with exactly 11 operands */
NOINLINE_NOIPA
void pattern_d_inline_asm_11ops(int *out, int a, int b, int c, int d, int e,
                                int f, int g, int h, int i, int j) {
    /* Directly create an RTL insn with 11 operands */
    asm volatile (
        "/* 11-operand asm pattern */\n\t"
        "add %[out], %[a], %[b]\n\t"
        "add %[out], %[out], %[c]\n\t"
        "add %[out], %[out], %[d]\n\t"
        "add %[out], %[out], %[e]\n\t"
        "add %[out], %[out], %[f]\n\t"
        "add %[out], %[out], %[g]\n\t"
        "add %[out], %[out], %[h]\n\t"
        "add %[out], %[out], %[i]\n\t"
        "add %[out], %[out], %[j]"
        : [out] "=r" (*out)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
}

/* Pattern E: Shuffle with variable mask (10+ operands when expanded) */
NOINLINE_NOIPA
v4sf pattern_e_variable_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                                unsigned mask0, unsigned mask1,
                                unsigned mask2, unsigned mask3) {
#if defined(__SSE__)
    /* Variable shuffle may expand to many operations */
    v4sf t1 = __builtin_ia32_shufps(a, b, mask0);
    v4sf t2 = __builtin_ia32_shufps(c, d, mask1);
    v4sf t3 = __builtin_ia32_shufps(t1, t2, mask2);
    return __builtin_ia32_shufps(t3, a + b + c + d, mask3);
#elif defined(__ARM_NEON)
    /* Use vtbl for variable shuffle */
    uint8x8x2_t tbl_a = { { vget_low_u8(vreinterpretq_u8_f32(a)), 
                            vget_high_u8(vreinterpretq_u8_f32(a)) } };
    uint8x8x2_t tbl_b = { { vget_low_u8(vreinterpretq_u8_f32(b)), 
                            vget_high_u8(vreinterpretq_u8_f32(b)) } };
    
    uint8x8_t idx = {mask0, mask1, mask2, mask3, mask0+4, mask1+4, mask2+4, mask3+4};
    uint8x8_t res_low = vtbl2_u8(tbl_a, idx);
    uint8x8_t res_high = vtbl2_u8(tbl_b, idx);
    
    return vreinterpretq_f32_u8(vcombine_u8(res_low, res_high));
#else
    v4sf result;
    unsigned masks[4] = {mask0, mask1, mask2, mask3};
    for (int i = 0; i < 4; i++) {
        unsigned idx = masks[i] & 0x3;
        result.f[i] = (idx < 2) ? a.f[idx] : b.f[idx-2];
    }
    return result;
#endif
}

/* Main test driver with runtime variability */
int main(int argc, char *argv[]) {
    volatile int seed = argc; /* Use argc for runtime variability */
    
    /* Initialize test vectors */
#if defined(__SSE__) || defined(__AVX__)
    v4sf va = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vd = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf ve = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vf = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf vg = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf vh = {29.0f, 30.0f, 31.0f, 32.0f};
    v4sf vi = {33.0f, 34.0f, 35.0f, 36.0f};
    v4sf vj = {37.0f, 38.0f, 39.0f, 40.0f};
    v4sf vk = {41.0f, 42.0f, 43.0f, 44.0f};
    v4sf vl = {45.0f, 46.0f, 47.0f, 48.0f};
#elif defined(__ARM_NEON)
    v4sf va = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vd = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf ve = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vf = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf vg = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf vh = {29.0f, 30.0f, 31.0f, 32.0f};
    v4sf vi = {33.0f, 34.0f, 35.0f, 36.0f};
    v4sf vj = {37.0f, 38.0f, 39.0f, 40.0f};
    v4sf vk = {41.0f, 42.0f, 43.0f, 44.0f};
    v4sf vl = {45.0f, 46.0f, 47.0f, 48.0f};
#else
    /* Scalar fallback */
    v4sf va = {{1.0f, 2.0f, 3.0f, 4.0f}};
    v4sf vb = {{5.0f, 6.0f, 7.0f, 8.0f}};
    v4sf vc = {{9.0f, 10.0f, 11.0f, 12.0f}};
    v4sf vd = {{13.0f, 14.0f, 15.0f, 16.0f}};
    v4sf ve = {{17.0f, 18.0f, 19.0f, 20.0f}};
    v4sf vf = {{21.0f, 22.0f, 23.0f, 24.0f}};
    v4sf vg = {{25.0f, 26.0f, 27.0f, 28.0f}};
    v4sf vh = {{29.0f, 30.0f, 31.0f, 32.0f}};
    v4sf vi = {{33.0f, 34.0f, 35.0f, 36.0f}};
    v4sf vj = {{37.0f, 38.0f, 39.0f, 40.0f}};
    v4sf vk = {{41.0f, 42.0f, 43.0f, 44.0f}};
    v4sf vl = {{45.0f, 46.0f, 47.0f, 48.0f}};
#endif
    
    float checksum = 0.0f;
    
    /* Execute different patterns based on seed to ensure multiple expansions */
    if (seed & 1) {
        v4sf r1 = pattern_a_blend_complex(va, vb, vc, vd, ve, vf, vg, vh,
                                         seed, seed>>1, seed>>2, seed>>3);
        checksum += ((float*)&r1)[0] + ((float*)&r1)[1];
    }
    
    if (seed & 2) {
        v4sf r2 = pattern_b_fma_chain(va, vb, vc, vd, ve, vf, vg, vh, vi, vj, vk, vl);
        checksum += ((float*)&r2)[2] + ((float*)&r2)[3];
    }
    
    if (seed & 4) {
        float r3 = pattern_c_scalarized_reduction(va, vb, vc, vd, ve, vf, vg, vh);
        checksum += r3;
    }
    
    if (seed & 8) {
        int asm_result;
        pattern_d_inline_asm_11ops(&asm_result, 
                                  seed, seed+1, seed+2, seed+3, seed+4,
                                  seed+5, seed+6, seed+7, seed+8, seed+9);
        checksum += asm_result;
    }
    
    if (seed & 16) {
        v4sf r5 = pattern_e_variable_shuffle(va, vb, vc, vd,
                                            seed & 0xFF, (seed>>8) & 0xFF,
                                            (seed>>16) & 0xFF, (seed>>24) & 0xFF);
        checksum += ((float*)&r5)[0] + ((float*)&r5)[3];
    }
    
    /* Use checksum to prevent dead code elimination */
    printf("Result checksum: %f\n", checksum);
    
    return (checksum > 0.0f) ? 0 : 1;
}
