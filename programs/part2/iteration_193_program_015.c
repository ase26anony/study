/* test_optabs_high_operand.c
 * 
 * This test targets GCC's optabs.cc expansion for operations with 10+ operands.
 * It uses various patterns to trigger the specific switch cases for 10 and 11 operands.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations from eliminating our test patterns */
#define KEEP_USED __attribute__((used))
#define NO_INLINE __attribute__((noinline, noipa))

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Pattern A: Vector blend with complex mask computation (10+ operands) */
NO_INLINE KEEP_USED
v4sf pattern_a_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                             v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE__
    /* Complex blend with mask computed from multiple comparisons */
    v4sf cmp1 = __builtin_ia32_cmpleps(a, b);
    v4sf cmp2 = __builtin_ia32_cmpgtps(c, d);
    v4sf cmp3 = __builtin_ia32_cmpeqps(e, f);
    v4sf cmp4 = __builtin_ia32_cmpneqps(g, h);
    
    /* Combine masks with logical operations */
    v4sf mask1 = __builtin_ia32_andps(cmp1, cmp2);
    v4sf mask2 = __builtin_ia32_orps(cmp3, cmp4);
    v4sf final_mask = __builtin_ia32_andnps(mask1, mask2);
    
    /* Blend using the complex mask */
    v4sf result = __builtin_ia32_blendvps(a, b, final_mask);
    return result;
#else
    /* Fallback for non-SSE targets */
    return a + b + c + d;
#endif
}

/* Pattern B: Fused multiply-add chain (creates deep expression tree) */
NO_INLINE KEEP_USED
float pattern_b_fma_chain(float a, float b, float c, float d,
                          float e, float f, float g, float h,
                          float i, float j, float k, float l) {
#ifdef __FMA__
    /* Chain of FMA operations that may expand to many operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    
    /* Combine results with more arithmetic */
    float result = __builtin_fmaf(t1, t2, __builtin_fmaf(t3, t4, a));
    return result;
#else
    /* Manual FMA simulation */
    return a * b + c + d * e + f + g * h + i + j * k + l;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization (10+ extracts) */
NO_INLINE KEEP_USED
float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually extract and sum all elements - creates many extract operations */
    float sum = 0.0f;
    
#ifdef __SSE__
    /* Extract each element (4 per vector, 4 vectors = 16 extracts) */
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
    /* Fallback using array access */
    float* p1 = (float*)&v1;
    float* p2 = (float*)&v2;
    float* p3 = (float*)&v3;
    float* p4 = (float*)&v4;
    
    for (int i = 0; i < 4; i++) {
        sum += p1[i] + p2[i] + p3[i] + p4[i];
    }
#endif
    
    return sum;
}

/* Pattern D: Complex shuffle with many immediate operands */
NO_INLINE KEEP_USED
v4sf pattern_d_complex_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                               int imm1, int imm2, int imm3, int imm4) {
#ifdef __SSE__
    /* Multiple shuffle operations with different immediates */
    v4sf s1 = __builtin_ia32_shufps(a, b, imm1);
    v4sf s2 = __builtin_ia32_shufps(c, d, imm2);
    
    /* Blend the shuffled results */
    v4sf blend_mask = (v4sf){imm3 & 1 ? -1.0f : 0.0f,
                             imm3 & 2 ? -1.0f : 0.0f,
                             imm3 & 4 ? -1.0f : 0.0f,
                             imm3 & 8 ? -1.0f : 0.0f};
    
    v4sf result = __builtin_ia32_blendvps(s1, s2, blend_mask);
    
    /* Final shuffle with computed control */
    int final_imm = (imm1 ^ imm2 ^ imm3 ^ imm4) & 0xFF;
    result = __builtin_ia32_shufps(result, result, final_imm);
    
    return result;
#else
    return a;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NO_INLINE KEEP_USED
int pattern_e_asm_11_operands(int a, int b, int c, int d, int e,
                              int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline asm with 11 total operands (1 output + 10 inputs) */
    asm volatile (
        /* Complex computation using all inputs */
        "imul %[a], %[b]\n\t"
        "add %[c], %[b]\n\t"
        "imul %[d], %[e]\n\t"
        "add %[f], %[e]\n\t"
        "imul %[g], %[h]\n\t"
        "add %[i], %[h]\n\t"
        "add %[b], %[e]\n\t"
        "add %[h], %[e]\n\t"
        "add %[j], %[e]\n\t"
        "mov %[e], %[out]"
        : [out] "=r" (result)          /* Output operand */
        : [a] "r" (a), [b] "r" (b),    /* 10 input operands */
          [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"                         /* Clobbers */
    );
    
    return result;
}

/* Pattern F: Vector comparison chain (10+ operands in expansion) */
NO_INLINE KEEP_USED
v4si pattern_f_vector_compare(v4sf a, v4sf b, v4sf c, v4sf d,
                              v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE2__
    /* Multiple comparisons combined with logical operations */
    v4si cmp1 = __builtin_ia32_cmpeqps(a, b);
    v4si cmp2 = __builtin_ia32_cmpgtps(c, d);
    v4si cmp3 = __builtin_ia32_cmpltps(e, f);
    v4si cmp4 = __builtin_ia32_cmpneqps(g, h);
    
    /* Complex combination */
    v4si tmp1 = __builtin_ia32_pand(cmp1, cmp2);
    v4si tmp2 = __builtin_ia32_por(cmp3, cmp4);
    v4si result = __builtin_ia32_pandn(tmp1, tmp2);
    
    /* Additional arithmetic to ensure expansion */
    result = __builtin_ia32_paddd(result, cmp1);
    result = __builtin_ia32_psubd(result, cmp2);
    
    return result;
#else
    v4si fallback = {0, 0, 0, 0};
    return fallback;
#endif
}

/* Main test driver that exercises all patterns */
int main(int argc, char** argv) {
    /* Use argc for runtime variability to ensure all paths are compiled */
    int test_case = argc > 1 ? atoi(argv[1]) % 6 : 0;
    
    /* Initialize test data */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf v5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf v6 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf v7 = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf v8 = {29.0f, 30.0f, 31.0f, 32.0f};
    
    float result_f = 0.0f;
    int result_i = 0;
    v4sf result_v4sf;
    v4si result_v4si;
    
    /* Execute different patterns based on test case */
    switch (test_case) {
        case 0:
            /* Pattern A: Complex blend */
            result_v4sf = pattern_a_blend_complex(v1, v2, v3, v4, v5, v6, v7, v8);
            sink = (int)result_v4sf[0];
            break;
            
        case 1:
            /* Pattern B: FMA chain */
            result_f = pattern_b_fma_chain(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f,
                                          7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f);
            sink = (int)result_f;
            break;
            
        case 2:
            /* Pattern C: Vector reduction */
            result_f = pattern_c_vector_reduction(v1, v2, v3, v4);
            sink = (int)result_f;
            break;
            
        case 3:
            /* Pattern D: Complex shuffle */
            result_v4sf = pattern_d_complex_shuffle(v1, v2, v3, v4, 0x1B, 0x27, 0x0F, 0x33);
            sink = (int)result_v4sf[0];
            break;
            
        case 4:
            /* Pattern E: Inline asm with 11 operands */
            result_i = pattern_e_asm_11_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            sink = result_i;
            break;
            
        case 5:
            /* Pattern F: Vector comparison chain */
            result_v4si = pattern_f_vector_compare(v1, v2, v3, v4, v5, v6, v7, v8);
            sink = result_v4si[0];
            break;
            
        default:
            sink = 42;
            break;
    }
    
    /* Ensure all patterns are referenced to prevent elimination */
    if (argc > 100) {  /* Never true, but compiler doesn't know */
        /* Force references to all patterns */
        pattern_a_blend_complex(v2, v1, v4, v3, v6, v5, v8, v7);
        pattern_b_fma_chain(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        pattern_c_vector_reduction(v2, v1, v4, v3);
        pattern_d_complex_shuffle(v2, v1, v4, v3, 0, 0, 0, 0);
        pattern_e_asm_11_operands(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        pattern_f_vector_compare(v2, v1, v4, v3, v6, v5, v8, v7);
    }
    
    printf("Test completed with sink = %d\n", sink);
    return 0;
}
