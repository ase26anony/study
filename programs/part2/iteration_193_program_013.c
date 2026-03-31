/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimizations that might eliminate our test patterns */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Use runtime input to prevent constant folding */
volatile int g_seed = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                int imm1, int imm2, int imm3, int imm4) {
#ifdef __SSE__
    /* __builtin_ia32_shufps typically expands to many operands */
    v4sf t1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf t2 = __builtin_ia32_shufps(c, d, imm2);
    v4sf t3 = __builtin_ia32_shufps(t1, t2, imm3);
    return __builtin_ia32_shufps(t3, a, imm4);
#else
    /* Fallback for non-SSE */
    return a + b + c + d;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE float pattern_b_fma_chain(float a, float b, float c, float d,
                                   float e, float f, float g, float h,
                                   float i, float j, float k, float l) {
#ifdef __FMA__
    /* Chain of FMA operations that may flatten to many operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    return __builtin_fmaf(t1, t2, __builtin_fmaf(t3, t4, a));
#else
    /* Manual FMA simulation */
    return a * b + c * d + e * f + g * h + i * j + k * l;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Extract each lane manually - creates many extract operations */
    float sum = 0.0f;
    
    /* Extract from v1 */
    sum += ((float*)&v1)[0] + ((float*)&v1)[1] + 
           ((float*)&v1)[2] + ((float*)&v1)[3];
    
    /* Extract from v2 */
    sum += ((float*)&v2)[0] + ((float*)&v2)[1] + 
           ((float*)&v2)[2] + ((float*)&v2)[3];
    
    /* Extract from v3 */
    sum += ((float*)&v3)[0] + ((float*)&v3)[1] + 
           ((float*)&v3)[2] + ((float*)&v3)[3];
    
    /* Extract from v4 */
    sum += ((float*)&v4)[0] + ((float*)&v4)[1] + 
           ((float*)&v4)[2] + ((float*)&v4)[3];
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE v4sf pattern_d_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                          v4sf mask1, v4sf mask2) {
#ifdef __SSE__
    /* Multiple comparisons and blends */
    v4sf cmp1 = __builtin_ia32_cmpps(a, b, 0);  /* EQ */
    v4sf cmp2 = __builtin_ia32_cmpps(c, d, 1);  /* LT */
    v4sf cmp3 = __builtin_ia32_cmpps(a, c, 2);  /* LE */
    
    /* Combine masks */
    v4sf mask = __builtin_ia32_andps(cmp1, cmp2);
    mask = __builtin_ia32_orps(mask, cmp3);
    
    /* Blend based on complex condition */
    v4sf t1 = __builtin_ia32_andps(a, mask);
    v4sf t2 = __builtin_ia32_andnps(mask, b);
    return __builtin_ia32_orps(t1, t2);
#else
    return a + b + c + d;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE int64_t pattern_e_multi_operand_asm(int64_t a, int64_t b, int64_t c,
                                             int64_t d, int64_t e, int64_t f,
                                             int64_t g, int64_t h, int64_t i,
                                             int64_t j) {
    int64_t result1, result2;
    
    /* Inline assembly with many operands - directly creates RTL with many ops */
    __asm__ volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "imul %[i], %[j]\n\t"
        : [result1] "=r" (result1), [result2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Vector permute with variable indices (ARM NEON specific) */
NOINLINE v4si pattern_f_neon_permute(v4si a, v4si b, v4si indices) {
#ifdef __ARM_NEON
    /* Use vector permute with many operands */
    return __builtin_shuffle(a, b, indices);
#else
    return a + b;
#endif
}

/* Main test driver */
int main(int argc, char *argv[]) {
    float result = 0.0f;
    
    /* Initialize with some values */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    v4si indices = {0, 4, 1, 5};  /* For shuffle */
    
    /* Use argc to add runtime variability */
    int selector = (argc > 1) ? atoi(argv[1]) % 6 : 0;
    
    switch (selector) {
        case 0:
            /* Pattern A: Complex shuffle */
            result = pattern_a_shuffle(vec1, vec2, vec3, vec4, 0x1B, 0x27, 0x39, 0x4E)[0];
            break;
            
        case 1:
            /* Pattern B: FMA chain */
            result = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
                                         7.7f, 8.8f, 9.9f, 10.1f, 11.1f, 12.2f);
            break;
            
        case 2:
            /* Pattern C: Vector reduction */
            result = pattern_c_vector_reduce(vec1, vec2, vec3, vec4);
            break;
            
        case 3:
            /* Pattern D: Conditional blend */
            result = pattern_d_conditional_blend(vec1, vec2, vec3, vec4, vec1, vec2)[0];
            break;
            
        case 4:
            /* Pattern E: Multi-operand inline assembly */
            result = (float)pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
            
        case 5:
            /* Pattern F: NEON permute */
            result = (float)pattern_f_neon_permute(ivec1, ivec2, indices)[0];
            break;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", result);
    
    return (int)result % 256;
}

/* Additional helper to ensure all patterns are compiled */
NOINLINE void ensure_all_patterns_compiled(void) {
    /* Force compilation of all patterns regardless of main execution path */
    volatile int dummy = 0;
    
    v4sf v1 = {0};
    v4sf v2 = {0};
    v4si iv1 = {0};
    v4si iv2 = {0};
    
    if (dummy) {
        pattern_a_shuffle(v1, v2, v1, v2, 0, 0, 0, 0);
        pattern_b_fma_chain(0,0,0,0,0,0,0,0,0,0,0,0);
        pattern_c_vector_reduce(v1, v2, v1, v2);
        pattern_d_conditional_blend(v1, v2, v1, v2, v1, v2);
        pattern_e_multi_operand_asm(0,0,0,0,0,0,0,0,0,0);
        pattern_f_neon_permute(iv1, iv2, iv2);
    }
}
