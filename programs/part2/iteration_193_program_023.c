/* test_optabs_coverage.c
 * 
 * This program is designed to trigger GCC's RTL expansion with 10-11 operands
 * to cover specific switch cases in optabs.cc (lines 8254-8263).
 * 
 * Compile with: gcc -O3 -march=native -fno-tree-vectorize -fprofile-arcs -ftest-coverage test_optabs_coverage.c -o test_optabs_coverage
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Volatile sink to prevent optimization */
static volatile int sink = 0;

/* Pattern 1: Complex vector shuffle with many operands */
NOINLINE static v4sf pattern1_shuffle_many_operands(float a, float b, float c, float d,
                                                    float e, float f, float g, float h,
                                                    int mask1, int mask2, int mask3, int mask4) {
    v4sf v1 = {a, b, c, d};
    v4sf v2 = {e, f, g, h};
    
#ifdef __SSE__
    /* __builtin_ia32_shufps expands to multiple RTL operands */
    v4sf result = __builtin_ia32_shufps(v1, v2, mask1);
    result = __builtin_ia32_shufps(result, v1, mask2);
    result = __builtin_ia32_shufps(result, v2, mask3);
    result = __builtin_ia32_shufps(result, result, mask4);
    return result;
#else
    /* Fallback for non-SSE targets */
    return v1 + v2;
#endif
}

/* Pattern 2: FMA chain creating deep expression tree */
NOINLINE static float pattern2_fma_chain(float a, float b, float c, float d, float e,
                                         float f, float g, float h, float i, float j) {
#ifdef __FMA__
    /* Chain of FMA operations that may flatten to many operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(t1, t2, t3);
    return __builtin_fmaf(t4, j, a + b + c + d);
#else
    /* Manual FMA simulation */
    return a * b + c + d * e + f + g * h + i + j;
#endif
}

/* Pattern 3: Vector reduction with explicit scalarization */
NOINLINE static float pattern3_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    float sum = 0.0f;
    
    /* Extract and sum each element - creates many extract operations */
#ifdef __SSE__
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
#else
    /* Fallback */
    float *p1 = (float*)&v1;
    float *p2 = (float*)&v2;
    float *p3 = (float*)&v3;
    float *p4 = (float*)&v4;
    for (int i = 0; i < 4; i++) {
        sum += p1[i] + p2[i] + p3[i] + p4[i];
    }
#endif
    
    return sum;
}

/* Pattern 4: Conditional vector operations with many comparisons */
NOINLINE static v4sf pattern4_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                                v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE__
    /* Multiple comparisons and blends */
    v4sf cmp1 = __builtin_ia32_cmpleps(a, b);
    v4sf cmp2 = __builtin_ia32_cmpgtps(c, d);
    v4sf cmp3 = __builtin_ia32_cmpeqps(e, f);
    v4sf cmp4 = __builtin_ia32_cmpneqps(g, h);
    
    v4sf blend1 = __builtin_ia32_blendvps(a, b, cmp1);
    v4sf blend2 = __builtin_ia32_blendvps(c, d, cmp2);
    v4sf blend3 = __builtin_ia32_blendvps(e, f, cmp3);
    v4sf blend4 = __builtin_ia32_blendvps(g, h, cmp4);
    
    /* Combine results */
    v4sf result = blend1 + blend2 + blend3 + blend4;
    return result;
#else
    return a + b + c + d + e + f + g + h;
#endif
}

/* Pattern 5: Inline assembly with exactly 11 operands */
NOINLINE static int pattern5_multi_operand_asm(int a, int b, int c, int d, int e,
                                               int f, int g, int h, int i, int j) {
    int result1, result2;
    
    /* Inline asm with 11 operands (10 inputs + 1 output) */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "imul %[b], %[a]\n\t"
        "mov %[out1], %[a]\n\t"
        "mov %[out2], %[b]"
        : [out1] "=r" (result1), [out2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern 6: ARM NEON specific - many operand vector operations */
NOINLINE static void pattern6_arm_neon_ops(int argc) {
#ifdef __ARM_NEON
    /* Use argc to create variability */
    float32x4_t v1 = {argc * 1.0f, argc * 2.0f, argc * 3.0f, argc * 4.0f};
    float32x4_t v2 = {argc * 5.0f, argc * 6.0f, argc * 7.0f, argc * 8.0f};
    float32x4_t v3 = {argc * 9.0f, argc * 10.0f, argc * 11.0f, argc * 12.0f};
    float32x4_t v4 = {argc * 13.0f, argc * 14.0f, argc * 15.0f, argc * 16.0f};
    
    /* Complex sequence of NEON operations */
    float32x4_t r1 = vaddq_f32(v1, v2);
    float32x4_t r2 = vmulq_f32(v3, v4);
    float32x4_t r3 = vmlaq_f32(r1, r2, v1);  /* r1 + r2 * v1 */
    float32x4_t r4 = vmlsq_f32(r3, v2, v4);  /* r3 - v2 * v4 */
    
    /* Extract and store to volatile to prevent optimization */
    float32_t lane0 = vgetq_lane_f32(r4, 0);
    float32_t lane1 = vgetq_lane_f32(r4, 1);
    float32_t lane2 = vgetq_lane_f32(r4, 2);
    float32_t lane3 = vgetq_lane_f32(r4, 3);
    
    sink = (int)(lane0 + lane1 + lane2 + lane3);
#endif
}

/* Pattern 7: AVX2 specific - 256-bit vector operations */
NOINLINE static void pattern7_avx2_multi_operand(int argc) {
#ifdef __AVX2__
    /* Create vectors with argc-dependent values */
    v8sf v1 = {argc * 1.0f, argc * 2.0f, argc * 3.0f, argc * 4.0f,
               argc * 5.0f, argc * 6.0f, argc * 7.0f, argc * 8.0f};
    v8sf v2 = {argc * 9.0f, argc * 10.0f, argc * 11.0f, argc * 12.0f,
               argc * 13.0f, argc * 14.0f, argc * 15.0f, argc * 16.0f};
    
    /* Complex AVX2 operations that may expand to many operands */
    v8sf r1 = v1 + v2;
    v8sf r2 = v1 * v2;
    v8sf r3 = r1 - r2;
    
    /* Permute operations */
    v8si mask = {0, 2, 4, 6, 1, 3, 5, 7};
    v8sf r4 = __builtin_ia32_permvarsf256(r3, mask);
    
    /* Blend operations */
    v8sf r5 = __builtin_ia32_blendps256(r1, r2, 0xAA);
    v8sf r6 = __builtin_ia32_blendps256(r3, r4, 0x55);
    
    v8sf final = r5 + r6;
    
    /* Store to volatile */
    float *p = (float*)&final;
    for (int i = 0; i < 8; i++) {
        sink += (int)p[i];
    }
#endif
}

/* Main test driver */
int main(int argc, char **argv) {
    float result = 0.0f;
    
    /* Use argc for runtime variability to ensure all paths are compiled */
    float base = (float)argc;
    
    /* Test Pattern 1: Complex shuffle */
    v4sf v1 = pattern1_shuffle_many_operands(
        base + 1.0f, base + 2.0f, base + 3.0f, base + 4.0f,
        base + 5.0f, base + 6.0f, base + 7.0f, base + 8.0f,
        argc % 256, (argc + 1) % 256, (argc + 2) % 256, (argc + 3) % 256
    );
    
    /* Test Pattern 2: FMA chain */
    result += pattern2_fma_chain(
        base + 1.0f, base + 2.0f, base + 3.0f, base + 4.0f, base + 5.0f,
        base + 6.0f, base + 7.0f, base + 8.0f, base + 9.0f, base + 10.0f
    );
    
    /* Test Pattern 3: Vector reduction */
    v4sf vec1 = {base + 1.0f, base + 2.0f, base + 3.0f, base + 4.0f};
    v4sf vec2 = {base + 5.0f, base + 6.0f, base + 7.0f, base + 8.0f};
    v4sf vec3 = {base + 9.0f, base + 10.0f, base + 11.0f, base + 12.0f};
    v4sf vec4 = {base + 13.0f, base + 14.0f, base + 15.0f, base + 16.0f};
    result += pattern3_vector_reduction(vec1, vec2, vec3, vec4);
    
    /* Test Pattern 4: Conditional blend */
    v4sf blend_result = pattern4_conditional_blend(vec1, vec2, vec3, vec4, 
                                                   vec1 + 1.0f, vec2 + 1.0f,
                                                   vec3 + 1.0f, vec4 + 1.0f);
    
    /* Test Pattern 5: Multi-operand inline assembly */
    int asm_result = pattern5_multi_operand_asm(
        argc, argc + 1, argc + 2, argc + 3, argc + 4,
        argc + 5, argc + 6, argc + 7, argc + 8, argc + 9
    );
    result += (float)asm_result;
    
    /* Test Pattern 6: ARM NEON operations */
    pattern6_arm_neon_ops(argc);
    
    /* Test Pattern 7: AVX2 operations */
    pattern7_avx2_multi_operand(argc);
    
    /* Use results to compute final checksum */
    float *v1p = (float*)&v1;
    float *blendp = (float*)&blend_result;
    
    float checksum = result;
    for (int i = 0; i < 4; i++) {
        checksum += v1p[i] + blendp[i];
    }
    
    /* Print checksum to prevent optimization */
    printf("Checksum: %f\n", checksum);
    
    return (int)(checksum * 1000.0f) % 256;
}
