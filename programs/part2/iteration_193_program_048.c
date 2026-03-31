/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */

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
static volatile int sink = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE USED v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                     int imm1, int imm2, int imm3, int imm4) {
#ifdef __SSE__
    /* Use SSE shuffle operations that expand to many RTL operands */
    v4sf t1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf t2 = __builtin_ia32_shufps(c, d, imm2);
    v4sf t3 = __builtin_ia32_shufps(t1, t2, imm3);
    v4sf result = __builtin_ia32_shufps(t3, a, imm4);
    
    /* Chain operations to increase operand count */
    result = result + t1 * t2;
    result = __builtin_ia32_shufps(result, b, imm1 & 0x0F);
    
    return result;
#else
    /* Fallback for non-SSE targets */
    return a + b + c + d;
#endif
}

/* Pattern B: Fused multiply-add chain */
NOINLINE USED float pattern_b_fma_chain(float a, float b, float c, float d,
                                        float e, float f, float g, float h,
                                        float i, float j, float k) {
#ifdef __FMA__
    /* Deep FMA chain that may flatten to many operands */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(t1, t2, t3);
    float result = __builtin_fma(t4, j, k);
    
    /* Additional arithmetic to prevent elimination */
    result = result + __builtin_fma(a, c, b);
    result = __builtin_fma(result, d, e);
    
    return result;
#else
    /* Manual FMA simulation */
    return a * b + c + d * e + f + g * h + i + j * k;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE USED float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    float sum = 0.0f;
    
    /* Manually extract and sum all elements - creates many extract operations */
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
    /* Fallback for non-SSE */
    float* f1 = (float*)&v1;
    float* f2 = (float*)&v2;
    float* f3 = (float*)&v3;
    float* f4 = (float*)&v4;
    
    for (int i = 0; i < 4; i++) {
        sum += f1[i] + f2[i] + f3[i] + f4[i];
    }
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE USED v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                                v4sf e, v4sf f) {
#ifdef __SSE__
    /* Multiple comparisons and blends */
    v4sf cmp1 = __builtin_ia32_cmpleps(a, b);
    v4sf cmp2 = __builtin_ia32_cmpnltps(c, d);
    v4sf cmp3 = __builtin_ia32_cmpneqps(e, f);
    
    /* Combine masks with logical operations */
    v4sf mask1 = __builtin_ia32_andps(cmp1, cmp2);
    v4sf mask2 = __builtin_ia32_orps(cmp3, cmp1);
    v4sf final_mask = __builtin_ia32_andnps(mask1, mask2);
    
    /* Blend based on complex mask */
    v4sf t1 = __builtin_ia32_blendvps(a, b, final_mask);
    v4sf t2 = __builtin_ia32_blendvps(c, d, final_mask);
    v4sf result = __builtin_ia32_blendvps(e, f, final_mask);
    
    /* Additional operations to increase operand count */
    result = result + t1 * t2;
    result = __builtin_ia32_shufps(result, final_mask, 0x1B);
    
    return result;
#else
    return a + b + c + d + e + f;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE USED int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                              int f, int g, int h, int i, int j) {
    int result1, result2;
    
    /* Inline asm with 11 total operands (2 outputs, 9 inputs) */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "mul %[r1], %[r1], %[e]\n\t"
        "mul %[r2], %[r2], %[f]\n\t"
        "add %[r1], %[r1], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "and %[r1], %[r1], %[i]\n\t"
        "or  %[r2], %[r2], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: ARM NEON specific - many lane operations */
NOINLINE USED float pattern_f_neon_operations(float a, float b, float c, float d,
                                              float e, float f, float g, float h) {
#ifdef __ARM_NEON
    typedef float32x4_t v4sf_neon;
    
    /* Construct vectors from many scalars */
    v4sf_neon v1 = {a, b, c, d};
    v4sf_neon v2 = {e, f, g, h};
    v4sf_neon v3 = {b, c, d, a};
    v4sf_neon v4 = {f, g, h, e};
    
    /* Multiple lane operations */
    v4sf_neon t1 = __builtin_neon_vmulq_f32(v1, v2);
    v4sf_neon t2 = __builtin_neon_vaddq_f32(v3, v4);
    v4sf_neon t3 = __builtin_neon_vmlaq_f32(t1, v2, v3);  /* FMA-like */
    
    /* Extract and combine many lanes */
    float r0 = __builtin_neon_vgetq_lane_f32(t1, 0);
    float r1 = __builtin_neon_vgetq_lane_f32(t1, 1);
    float r2 = __builtin_neon_vgetq_lane_f32(t1, 2);
    float r3 = __builtin_neon_vgetq_lane_f32(t1, 3);
    
    float r4 = __builtin_neon_vgetq_lane_f32(t2, 0);
    float r5 = __builtin_neon_vgetq_lane_f32(t2, 1);
    float r6 = __builtin_neon_vgetq_lane_f32(t2, 2);
    float r7 = __builtin_neon_vgetq_lane_f32(t2, 3);
    
    return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
#else
    return a + b + c + d + e + f + g + h;
#endif
}

/* Main test driver */
int main(int argc, char** argv) {
    float result = 0.0f;
    
    /* Use argc to add runtime variability */
    int selector = argc > 1 ? atoi(argv[1]) % 6 : 0;
    
    /* Initialize test data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Execute different patterns based on selector */
    switch (selector) {
        case 0:
            result = pattern_a_shuffle(vec1, vec2, vec3, vec4, 0x1B, 0x27, 0x39, 0x4E);
            break;
        case 1:
            result = pattern_b_fma_chain(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f,
                                         7.0f, 8.0f, 9.0f, 10.0f, 11.0f);
            break;
        case 2:
            result = pattern_c_vector_reduce(vec1, vec2, vec3, vec4);
            break;
        case 3:
            result = ((float*)&pattern_d_conditional_select(vec1, vec2, vec3, vec4, 
                                                           vec1, vec2))[0];
            break;
        case 4:
            result = (float)pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
        case 5:
            result = pattern_f_neon_operations(1.0f, 2.0f, 3.0f, 4.0f,
                                               5.0f, 6.0f, 7.0f, 8.0f);
            break;
        default:
            result = 42.0f;
    }
    
    /* Use result to prevent optimization */
    sink = (int)result;
    printf("Result: %f\n", result);
    
    return sink != 0 ? 0 : 1;
}
