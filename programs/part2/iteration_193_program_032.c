/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for different architectures */
#ifdef __SSE__
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
#endif

#ifdef __AVX__
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
typedef float32x4_t v4sf;
typedef int32x4_t v4si;
#endif

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent optimization */
static volatile int sink = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE static v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                       int m1, int m2, int m3, int m4) {
#ifdef __SSE__
    /* Create complex shuffle pattern - may expand to many operands */
    v4sf t1 = __builtin_ia32_shufps(a, b, m1);
    v4sf t2 = __builtin_ia32_shufps(c, d, m2);
    v4sf t3 = __builtin_ia32_shufps(t1, t2, m3);
    v4sf result = __builtin_ia32_shufps(t3, a, m4);
    
    /* Mix with arithmetic to prevent elimination */
    result = result + t1 * 0.5f;
    result = result - t2 * 0.25f;
    
    return result;
#else
    (void)a; (void)b; (void)c; (void)d;
    (void)m1; (void)m2; (void)m3; (void)m4;
    v4sf result = {0};
    return result;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE static float pattern_b_fma_chain(float a, float b, float c, 
                                          float d, float e, float f,
                                          float g, float h, float i,
                                          float j, float k) {
#ifdef __FP_FAST_FMA
    /* Chain of FMA operations - may flatten to many operands */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(t1, t2, t3);
    float result = __builtin_fma(t4, j, k);
    
    /* Additional arithmetic to increase operand count */
    result = result + a * b * c;
    result = result - d * e * f;
    
    return result;
#else
    /* Manual FMA simulation */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = t1 * t2 + t3;
    float result = t4 * j + k;
    
    /* Complex expression that may expand to many operands */
    result = result + (a * b * c) / (d + 1.0f);
    result = result - (e * f * g) / (h + 1.0f);
    
    return result;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c_vector_reduction(v4sf v1, v4sf v2, 
                                                 v4sf v3, v4sf v4) {
    float sum = 0.0f;
    
#ifdef __SSE__
    /* Extract and sum all elements - creates many extract operations */
    sum += ((float*)&v1)[0] + ((float*)&v1)[1] + ((float*)&v1)[2] + ((float*)&v1)[3];
    sum += ((float*)&v2)[0] + ((float*)&v2)[1] + ((float*)&v2)[2] + ((float*)&v2)[3];
    sum += ((float*)&v3)[0] + ((float*)&v3)[1] + ((float*)&v3)[2] + ((float*)&v3)[3];
    sum += ((float*)&v4)[0] + ((float*)&v4)[1] + ((float*)&v4)[2] + ((float*)&v4)[3];
    
    /* Additional complex operations */
    sum = sum * 1.5f - 2.0f;
    sum = sum / 3.0f + 4.0f;
#elif defined(__ARM_NEON)
    /* NEON extraction */
    sum += vgetq_lane_f32(v1, 0) + vgetq_lane_f32(v1, 1) +
           vgetq_lane_f32(v1, 2) + vgetq_lane_f32(v1, 3);
    sum += vgetq_lane_f32(v2, 0) + vgetq_lane_f32(v2, 1) +
           vgetq_lane_f32(v2, 2) + vgetq_lane_f32(v2, 3);
    sum += vgetq_lane_f32(v3, 0) + vgetq_lane_f32(v3, 1) +
           vgetq_lane_f32(v3, 2) + vgetq_lane_f32(v3, 3);
    sum += vgetq_lane_f32(v4, 0) + vgetq_lane_f32(v4, 1) +
           vgetq_lane_f32(v4, 2) + vgetq_lane_f32(v4, 3);
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE static v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c,
                                                  v4sf d, v4sf mask1,
                                                  v4sf mask2, v4sf mask3) {
#ifdef __SSE__
    /* Complex conditional logic that may expand to many operands */
    v4sf cmp1 = a > b;
    v4sf cmp2 = c < d;
    v4sf cmp3 = (a + b) == (c - d);
    
    /* Blend operations based on multiple conditions */
    v4sf t1 = __builtin_ia32_blendps(a, b, 0x5);  /* 0101 pattern */
    v4sf t2 = __builtin_ia32_blendps(c, d, 0xA);  /* 1010 pattern */
    
    /* More complex blending with computed masks */
    v4sf result = t1;
    result = (cmp1 & mask1) ? t2 : result;
    result = (cmp2 & mask2) ? (t1 + t2) : result;
    result = (cmp3 & mask3) ? (t1 - t2) : result;
    
    /* Final arithmetic mix */
    result = result * 2.0f + a - b;
    
    return result;
#else
    (void)a; (void)b; (void)c; (void)d;
    (void)mask1; (void)mask2; (void)mask3;
    v4sf result = {0};
    return result;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int pattern_e_multi_operand_asm(int a, int b, int c, int d,
                                                int e, int f, int g, int h,
                                                int i, int j, int k) {
    int result1, result2, result3;
    
    /* Inline assembly with many input/output operands */
    asm volatile (
        /* Complex multi-operand operation */
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r2], %[d], %[e]\n\t"
        "add %[r2], %[r2], %[f]\n\t"
        "add %[r3], %[g], %[h]\n\t"
        "add %[r3], %[r3], %[i]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[out], %[r1], %[r3]\n\t"
        "add %[out], %[out], %[j]\n\t"
        "sub %[out], %[out], %[k]"
        
        : [out] "=r" (result1), [r1] "=&r" (result2), 
          [r2] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1;
}

/* Pattern F: Complex builtin with immediate and multiple vectors */
NOINLINE static v4sf pattern_f_complex_builtin(v4sf a, v4sf b, v4sf c,
                                               v4sf d, int imm1, int imm2,
                                               int imm3, int imm4) {
#ifdef __AVX__
    /* AVX builtins that take multiple vectors and immediates */
    v4sf t1 = __builtin_ia32_vpermilps(a, imm1);
    v4sf t2 = __builtin_ia32_vpermilps(b, imm2);
    v4sf t3 = __builtin_ia32_vpermilps(c, imm3);
    v4sf t4 = __builtin_ia32_vpermilps(d, imm4);
    
    /* Complex blend chain */
    v4sf r1 = __builtin_ia32_blendps256(t1, t2, 0x33);
    v4sf r2 = __builtin_ia32_blendps256(t3, t4, 0xCC);
    v4sf result = __builtin_ia32_blendps256(r1, r2, 0xAA);
    
    return result;
#elif defined(__SSE4_1__)
    /* SSE4.1 blend with variable masks */
    v4sf t1 = __builtin_ia32_blendps(a, b, imm1);
    v4sf t2 = __builtin_ia32_blendps(c, d, imm2);
    v4sf result = __builtin_ia32_blendps(t1, t2, imm3);
    
    /* Additional shuffle */
    result = __builtin_ia32_shufps(result, result, imm4);
    
    return result;
#else
    (void)a; (void)b; (void)c; (void)d;
    (void)imm1; (void)imm2; (void)imm3; (void)imm4;
    v4sf result = {0};
    return result;
#endif
}

/* Main test driver */
int main(int argc, char *argv[]) {
    float total = 0.0f;
    
    /* Initialize test data with some variability */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf mask = {0xFFFFFFFF, 0, 0xFFFFFFFF, 0};
    
    /* Use argc to select different patterns (ensures all get compiled) */
    switch (argc % 6) {
        case 0:
            /* Pattern A - Shuffle with many operands */
            {
                v4sf result = pattern_a_shuffle(vec1, vec2, vec3, vec4,
                                               0x1B, 0x4E, 0x93, 0x27);
                total += ((float*)&result)[0] + ((float*)&result)[1] +
                        ((float*)&result)[2] + ((float*)&result)[3];
            }
            break;
            
        case 1:
            /* Pattern B - FMA chain with 11 scalar operands */
            {
                float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
                float f5 = 5.5f, f6 = 6.6f, f7 = 7.7f, f8 = 8.8f;
                float f9 = 9.9f, f10 = 10.1f, f11 = 11.1f;
                
                float result = pattern_b_fma_chain(f1, f2, f3, f4, f5,
                                                  f6, f7, f8, f9, f10, f11);
                total += result;
            }
            break;
            
        case 2:
            /* Pattern C - Vector reduction with many extracts */
            {
                float result = pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
                total += result;
            }
            break;
            
        case 3:
            /* Pattern D - Conditional select with many comparisons */
            {
                v4sf result = pattern_d_conditional_select(vec1, vec2, vec3, vec4,
                                                          mask, mask, mask);
                total += ((float*)&result)[0] + ((float*)&result)[1] +
                        ((float*)&result)[2] + ((float*)&result)[3];
            }
            break;
            
        case 4:
            /* Pattern E - Inline assembly with 11 operands */
            {
                int result = pattern_e_multi_operand_asm(1, 2, 3, 4, 5,
                                                        6, 7, 8, 9, 10, 11);
                total += (float)result;
            }
            break;
            
        case 5:
            /* Pattern F - Complex builtin with many immediates */
            {
                v4sf result = pattern_f_complex_builtin(vec1, vec2, vec3, vec4,
                                                       0x1B, 0x4E, 0x93, 0x27);
                total += ((float*)&result)[0] + ((float*)&result)[1] +
                        ((float*)&result)[2] + ((float*)&result)[3];
            }
            break;
    }
    
    /* Use result to prevent optimization */
    sink = (int)total;
    printf("Result: %f\n", total);
    
    return (total > 0) ? 0 : 1;
}
