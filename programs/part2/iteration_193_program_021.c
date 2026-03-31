/* test_optabs_high_operand_count.c
 * 
 * This test aims to cover the 10 and 11 operand switch cases in optabs.cc
 * by generating code that requires expansion of operations with many operands.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
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

/* Volatile to prevent dead code elimination */
static volatile int g_volatile_sink = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE static v4sf pattern_a_shuffle_many_operands(v4sf a, v4sf b, v4sf c, v4sf d,
                                                     int imm1, int imm2, int imm3, int imm4)
{
    /* This should expand to many operands due to multiple shuffles and blends */
#ifdef __SSE__
    v4sf t1 = _mm_shuffle_ps(a, b, imm1);
    v4sf t2 = _mm_shuffle_ps(c, d, imm2);
    v4sf t3 = _mm_shuffle_ps(t1, t2, imm3);
    v4sf t4 = _mm_shuffle_ps(b, c, imm4);
    return _mm_shuffle_ps(t3, t4, (imm1 ^ imm2) & 0xFF);
#else
    /* Fallback implementation */
    v4sf t1 = __builtin_shuffle(a, b, (v4si){imm1 & 3, (imm1 >> 2) & 3, 
                                            (imm1 >> 4) & 3, (imm1 >> 6) & 3});
    v4sf t2 = __builtin_shuffle(c, d, (v4si){imm2 & 3, (imm2 >> 2) & 3, 
                                            (imm2 >> 4) & 3, (imm2 >> 6) & 3});
    v4sf t3 = __builtin_shuffle(t1, t2, (v4si){imm3 & 3, (imm3 >> 2) & 3, 
                                              (imm3 >> 4) & 3, (imm3 >> 6) & 3});
    v4sf t4 = __builtin_shuffle(b, c, (v4si){imm4 & 3, (imm4 >> 2) & 3, 
                                            (imm4 >> 4) & 3, (imm4 >> 6) & 3});
    int imm5 = (imm1 ^ imm2) & 0xFF;
    return __builtin_shuffle(t3, t4, (v4si){imm5 & 3, (imm5 >> 2) & 3, 
                                           (imm5 >> 4) & 3, (imm5 >> 6) & 3});
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE static float pattern_b_fma_chain(float a, float b, float c, float d,
                                          float e, float f, float g, float h,
                                          float i, float j, float k, float l)
{
    /* Chain of FMAs - may be expanded to many operands */
#ifdef __FMA__
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    
    float t5 = __builtin_fmaf(t1, t2, t3);
    float t6 = __builtin_fmaf(t4, t1, t2);
    
    return __builtin_fmaf(t5, t6, t1 + t2 + t3 + t4);
#else
    /* Manual FMA emulation - still creates many operands */
    return (((a * b + c) * (d * e + f) + (g * h + i)) *
            ((j * k + l) * (a * b + c) + (d * e + f)) +
            ((a * b + c) + (d * e + f) + (g * h + i) + (j * k + l)));
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4)
{
    /* Extract each element and sum - creates many extract operations */
    float sum = 0.0f;
    
    /* Extract all elements from 4 vectors = 16 extractions */
    for (int i = 0; i < 4; i++) {
        sum += ((float*)&v1)[i];
        sum += ((float*)&v2)[i];
        sum += ((float*)&v3)[i];
        sum += ((float*)&v4)[i];
    }
    
    /* Additional arithmetic to create more operands */
    sum = sum * 2.0f - sum / 2.0f + sum * 1.5f - sum / 1.5f;
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE static v4sf pattern_d_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                                 v4sf mask1, v4sf mask2, v4sf mask3)
{
    /* Complex conditional blending - may expand to many operands */
#ifdef __SSE__
    v4sf cmp1 = _mm_cmplt_ps(a, b);
    v4sf cmp2 = _mm_cmpgt_ps(c, d);
    v4sf cmp3 = _mm_cmpeq_ps(mask1, mask2);
    
    v4sf and1 = _mm_and_ps(cmp1, cmp2);
    v4sf and2 = _mm_and_ps(cmp3, mask3);
    v4sf or1 = _mm_or_ps(and1, and2);
    
    /* Blend based on complex mask */
    v4sf t1 = _mm_blendv_ps(a, b, or1);
    v4sf t2 = _mm_blendv_ps(c, d, _mm_xor_ps(or1, mask1));
    
    return _mm_add_ps(t1, t2);
#else
    /* Manual implementation */
    v4sf cmp1 = (a < b);
    v4sf cmp2 = (c > d);
    v4sf cmp3 = (mask1 == mask2);
    
    v4sf and1 = cmp1 & cmp2;
    v4sf and2 = cmp3 & mask3;
    v4sf or1 = and1 | and2;
    
    v4sf t1 = __builtin_shuffle(a, b, (v4si){(or1[0] != 0.0f) ? 4 : 0,
                                            (or1[1] != 0.0f) ? 5 : 1,
                                            (or1[2] != 0.0f) ? 6 : 2,
                                            (or1[3] != 0.0f) ? 7 : 3});
    
    v4sf xor_mask = or1 ^ mask1;
    v4sf t2 = __builtin_shuffle(c, d, (v4si){(xor_mask[0] != 0.0f) ? 4 : 0,
                                            (xor_mask[1] != 0.0f) ? 5 : 1,
                                            (xor_mask[2] != 0.0f) ? 6 : 2,
                                            (xor_mask[3] != 0.0f) ? 7 : 3});
    
    return t1 + t2;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int pattern_e_asm_many_operands(int a, int b, int c, int d, int e,
                                                int f, int g, int h, int i, int j)
{
    int result1, result2;
    
    /* Inline assembly with 11 total operands (2 outputs, 9 inputs) */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r1], %[r1], %[d]\n\t"
        "add %[r2], %[e], %[f]\n\t"
        "add %[r2], %[r2], %[g]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "sub %[r1], %[r1], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Complex vector multiply-highpart simulation */
NOINLINE static v4si pattern_f_mult_highpart(v4si a, v4si b, v4si c, v4si d)
{
    /* Simulate operations that might use expand_mult_highpart */
    v4si t1 = a * b;
    v4si t2 = c * d;
    v4si t3 = (a >> 16) * (b >> 16);
    v4si t4 = (c >> 16) * (d >> 16);
    
    /* Complex combination - may trigger high-part multiplication expansion */
    v4si r1 = t1 + t2;
    v4si r2 = t3 + t4;
    v4si r3 = (t1 >> 8) + (t2 >> 8);
    v4si r4 = (t3 >> 8) + (t4 >> 8);
    
    return (r1 << 1) + (r2 << 2) + (r3 << 3) + (r4 << 4);
}

/* Main test driver */
int main(int argc, char *argv[]) {
    float checksum = 0.0f;
    
    /* Initialize test data with some variability based on argc */
    v4sf vec1 = {1.0f + argc, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f + argc, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f + argc, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f + argc};
    
    v4si ivec1 = {1 + argc, 2, 3, 4};
    v4si ivec2 = {5, 6 + argc, 7, 8};
    v4si ivec3 = {9, 10, 11 + argc, 12};
    v4si ivec4 = {13, 14, 15, 16 + argc};
    
    /* Execute all patterns to ensure they're compiled */
    if (argc > 1) {
        /* Pattern A: Complex shuffle */
        v4sf res_a = pattern_a_shuffle_many_operands(vec1, vec2, vec3, vec4,
                                                     argc & 0xFF, 
                                                     (argc >> 8) & 0xFF,
                                                     (argc >> 16) & 0xFF,
                                                     (argc >> 24) & 0xFF);
        checksum += res_a[0] + res_a[1] + res_a[2] + res_a[3];
        
        /* Pattern B: FMA chain */
        float res_b = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f,
                                          5.5f, 6.6f, 7.7f, 8.8f,
                                          9.9f, 10.1f, 11.11f, 12.12f);
        checksum += res_b;
        
        /* Pattern C: Vector reduction */
        float res_c = pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
        checksum += res_c;
        
        /* Pattern D: Conditional blend */
        v4sf mask1 = {argc & 1 ? 1.0f : 0.0f, 
                      argc & 2 ? 1.0f : 0.0f,
                      argc & 4 ? 1.0f : 0.0f,
                      argc & 8 ? 1.0f : 0.0f};
        v4sf res_d = pattern_d_conditional_blend(vec1, vec2, vec3, vec4,
                                                 mask1, vec1, vec2);
        checksum += res_d[0] + res_d[1] + res_d[2] + res_d[3];
        
        /* Pattern E: Inline assembly with many operands */
        int res_e = pattern_e_asm_many_operands(argc, argc+1, argc+2, argc+3,
                                                argc+4, argc+5, argc+6,
                                                argc+7, argc+8, argc+9);
        checksum += (float)res_e;
        
        /* Pattern F: Multiply highpart simulation */
        v4si res_f = pattern_f_mult_highpart(ivec1, ivec2, ivec3, ivec4);
        checksum += (float)(res_f[0] + res_f[1] + res_f[2] + res_f[3]);
    }
    
    /* Use checksum to prevent optimization */
    g_volatile_sink = (int)checksum;
    
    printf("Result: %f\n", checksum);
    return (int)checksum & 0xFF;
}
