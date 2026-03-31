/* test_optabs_high_operand_count.c
 * 
 * This program is designed to trigger the 10 and 11 operand switch cases
 * in GCC's optabs.cc expansion routines by creating complex operations
 * that require many RTL operands during expansion.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE static v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                       int imm1, int imm2, int imm3, int imm4)
{
    /* This should expand to many operands as shuffle operations
     * often decompose into multiple RTL instructions */
#ifdef __SSE__
    v4sf t1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf t2 = __builtin_ia32_shufps(c, d, imm2);
    v4sf t3 = __builtin_ia32_shufps(t1, t2, imm3);
    return __builtin_ia32_shufps(t3, a, imm4);
#else
    /* Fallback for non-SSE targets - still creates many operands */
    v4sf t1 = {a[imm1 & 3], a[(imm1 >> 2) & 3], b[(imm1 >> 4) & 3], b[(imm1 >> 6) & 3]};
    v4sf t2 = {c[imm2 & 3], c[(imm2 >> 2) & 3], d[(imm2 >> 4) & 3], d[(imm2 >> 6) & 3]};
    v4sf t3 = {t1[imm3 & 3], t1[(imm3 >> 2) & 3], t2[(imm3 >> 4) & 3], t2[(imm3 >> 6) & 3]};
    return t3;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE static float pattern_b_fma_chain(float a, float b, float c, float d,
                                          float e, float f, float g, float h,
                                          float i, float j, float k, float l)
{
    /* Chain of FMAs that should flatten into many operands */
#ifdef __FP_FAST_FMA
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(j, k, l);
    
    /* Create dependency chain with many operands */
    float r1 = __builtin_fma(t1, t2, t3);
    float r2 = __builtin_fma(t4, t1, t2);
    return __builtin_fma(r1, r2, t4);
#else
    /* Manual FMA emulation - still creates many operands */
    return (((a * b + c) * (d * e + f) + (g * h + i)) * 
            ((j * k + l) * (a * b + c) + (d * e + f)) + 
            (j * k + l));
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4)
{
    /* Extract each element and sum - creates many extract operations */
    float sum = 0.0f;
    
    /* Unrolled extraction - each extract is a separate operation */
    sum += v1[0] + v1[1] + v1[2] + v1[3];
    sum += v2[0] + v2[1] + v2[2] + v2[3];
    sum += v3[0] + v3[1] + v3[2] + v3[3];
    sum += v4[0] + v4[1] + v4[2] + v4[3];
    
    /* Additional arithmetic to create more operands */
    sum = sum * 2.0f - (v1[0] * v2[1]) + (v3[2] * v4[3]);
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE static v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                                  v4sf e, v4sf f, v4sf mask1,
                                                  v4sf mask2, v4sf mask3)
{
    /* Complex conditional selection that may expand to many RTL operands */
#ifdef __SSE__
    v4sf cmp1 = __builtin_ia32_cmpps(a, b, 0);  /* EQ */
    v4sf cmp2 = __builtin_ia32_cmpps(c, d, 1);  /* LT */
    v4sf cmp3 = __builtin_ia32_cmpps(e, f, 2);  /* LE */
    
    /* Combine masks - each operation adds operands */
    v4sf combined_mask = __builtin_ia32_andps(cmp1, mask1);
    combined_mask = __builtin_ia32_orps(combined_mask, cmp2);
    combined_mask = __builtin_ia32_andps(combined_mask, mask2);
    combined_mask = __builtin_ia32_orps(combined_mask, cmp3);
    combined_mask = __builtin_ia32_andps(combined_mask, mask3);
    
    /* Select based on complex mask */
    v4sf result = __builtin_ia32_andps(a, combined_mask);
    v4sf alt = __builtin_ia32_andnotps(combined_mask, b);
    return __builtin_ia32_orps(result, alt);
#else
    /* Manual implementation */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        int cond = (a[i] == b[i]) && mask1[i] ||
                   (c[i] < d[i]) || mask2[i] ||
                   (e[i] <= f[i]) && mask3[i];
        result[i] = cond ? a[i] : b[i];
    }
    return result;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                                int f, int g, int h, int i, int j)
{
    int result1, result2;
    
    /* Inline asm with 11 total operands (2 outputs, 9 inputs) */
    asm volatile (
        "add %[a], %[b], %[t1]\n\t"
        "add %[c], %[d], %[t2]\n\t"
        "mul %[t1], %[t2], %[t3]\n\t"
        "add %[e], %[f], %[t4]\n\t"
        "add %[g], %[h], %[t5]\n\t"
        "mul %[t4], %[t5], %[t6]\n\t"
        "add %[t3], %[t6], %[r1]\n\t"
        "add %[i], %[j], %[r2]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "t1", "t2", "t3", "t4", "t5", "t6"
    );
    
    return result1 + result2;
}

/* Pattern F: Complex builtin with immediate and many vector arguments */
NOINLINE static v4sf pattern_f_complex_builtin(v4sf a, v4sf b, v4sf c, v4sf d,
                                               v4sf e, v4sf f, int imm)
{
#ifdef __AVX__
    /* AVX blend with complex pattern - may require many operands */
    v4sf t1 = __builtin_ia32_blendps256(a, b, imm & 0xFF);
    v4sf t2 = __builtin_ia32_blendps256(c, d, (imm >> 8) & 0xFF);
    v4sf t3 = __builtin_ia32_blendps256(e, f, (imm >> 16) & 0xFF);
    
    /* Additional operations to increase operand count */
    v4sf r1 = __builtin_ia32_addps(t1, t2);
    v4sf r2 = __builtin_ia32_subps(t3, t1);
    v4sf r3 = __builtin_ia32_mulps(r1, r2);
    
    return __builtin_ia32_blendps256(r3, a, imm & 0x0F);
#elif defined(__SSE__)
    /* SSE version with multiple blends */
    v4sf t1 = __builtin_ia32_blendps(a, b, imm & 0x0F);
    v4sf t2 = __builtin_ia32_blendps(c, d, (imm >> 4) & 0x0F);
    v4sf t3 = __builtin_ia32_blendps(e, f, (imm >> 8) & 0x0F);
    
    v4sf r1 = __builtin_ia32_addps(t1, t2);
    v4sf r2 = __builtin_ia32_subps(t3, t1);
    return __builtin_ia32_mulps(r1, r2);
#else
    /* Manual blend */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        int mask = (imm >> (i * 2)) & 0x3;
        v4sf src[] = {a, b, c, d, e, f};
        result[i] = src[mask % 6][i];
    }
    return result;
#endif
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argc for runtime variability to ensure all paths are compiled */
    int test_case = argc > 1 ? atoi(argv[1]) % 6 : 0;
    
    /* Initialize test data */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf mask = {0xFFFFFFFF, 0, 0xFFFFFFFF, 0};
    
    float result_float = 0.0f;
    v4sf result_vec = {0};
    int result_int = 0;
    
    switch (test_case) {
        case 0:
            /* Pattern A: Shuffle with many operands */
            result_vec = pattern_a_shuffle(v1, v2, v3, v4, 0x1B, 0x27, 0x39, 0x4E);
            sink = (int)result_vec[0];
            break;
            
        case 1:
            /* Pattern B: FMA chain with many operands */
            result_float = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
                                              7.7f, 8.8f, 9.9f, 10.1f, 11.1f, 12.2f);
            sink = (int)result_float;
            break;
            
        case 2:
            /* Pattern C: Vector reduction with many extracts */
            result_float = pattern_c_vector_reduce(v1, v2, v3, v4);
            sink = (int)result_float;
            break;
            
        case 3:
            /* Pattern D: Conditional select with many comparisons */
            result_vec = pattern_d_conditional_select(v1, v2, v3, v4, v1, v2, mask, mask, mask);
            sink = (int)result_vec[0];
            break;
            
        case 4:
            /* Pattern E: Inline asm with 11 operands */
            result_int = pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            sink = result_int;
            break;
            
        case 5:
            /* Pattern F: Complex builtin with many vector args */
            result_vec = pattern_f_complex_builtin(v1, v2, v3, v4, v1, v2, 0x123456);
            sink = (int)result_vec[0];
            break;
    }
    
    /* Return something based on results to prevent optimization */
    return sink % 256;
}
