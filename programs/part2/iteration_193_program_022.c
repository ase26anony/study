/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
#include <emmintrin.h>
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

/* Volatile sink to prevent elimination */
static volatile int sink;

/* Pattern A: Vector blend with complex mask (x86 SSE/AVX) */
NOINLINE static v4sf pattern_a_blend_many_operands(
    v4sf a, v4sf b, v4sf c, v4sf d,
    int m1, int m2, int m3, int m4,
    float f1, float f2)
{
#ifdef __SSE__
    /* Create complex mask from multiple inputs */
    __m128 mask = _mm_set_ps(
        (m1 & 1) ? f1 : f2,
        (m2 & 2) ? f2 : f1,
        (m3 & 4) ? f1 : f2,
        (m4 & 8) ? f2 : f1
    );
    
    /* Multiple blend operations chained */
    __m128 t1 = _mm_blend_ps(a, b, m1 & 0xF);
    __m128 t2 = _mm_blend_ps(c, d, m2 & 0xF);
    __m128 t3 = _mm_blend_ps(t1, t2, m3 & 0xF);
    __m128 result = _mm_blend_ps(t3, mask, m4 & 0xF);
    
    /* Additional arithmetic to create dependencies */
    result = _mm_add_ps(result, _mm_mul_ps(a, b));
    result = _mm_sub_ps(result, _mm_mul_ps(c, d));
    
    return result;
#else
    /* Fallback for non-SSE */
    return a + b + c + d;
#endif
}

/* Pattern B: FMA chain creating deep expression tree */
NOINLINE static float pattern_b_fma_chain(
    float a1, float b1, float c1,
    float a2, float b2, float c2,
    float a3, float b3, float c3,
    float a4, float b4)
{
#ifdef __FMA__
    /* Chain of FMA operations - may expand to many operands */
    float t1 = __builtin_fmaf(a1, b1, c1);
    float t2 = __builtin_fmaf(a2, b2, c2);
    float t3 = __builtin_fmaf(a3, b3, c3);
    float t4 = __builtin_fmaf(a4, b4, t1);
    float result = __builtin_fmaf(t2, t3, t4);
    
    /* Additional operations to prevent simplification */
    result = __builtin_fmaf(result, a1, b1);
    result = __builtin_fmaf(result, a2, -b2);
    
    return result;
#else
    /* Manual FMA emulation */
    return a1 * b1 + c1 + a2 * b2 + c2 + a3 * b3 + c3 + a4 * b4;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c_vector_reduction(
    v4sf v1, v4sf v2, v4sf v3, v4sf v4,
    int i1, int i2, int i3, int i4)
{
    /* Extract each element manually - creates many extract operations */
    float sum = 0.0f;
    
#ifdef __SSE__
    /* Force extraction of each element */
    float e1 = ((float*)&v1)[i1 & 3];
    float e2 = ((float*)&v1)[i2 & 3];
    float e3 = ((float*)&v2)[i3 & 3];
    float e4 = ((float*)&v2)[i4 & 3];
    float e5 = ((float*)&v3)[i1 & 3];
    float e6 = ((float*)&v3)[i2 & 3];
    float e7 = ((float*)&v4)[i3 & 3];
    float e8 = ((float*)&v4)[i4 & 3];
    
    /* Complex reduction chain */
    sum = e1 + e2 + e3 + e4 + e5 + e6 + e7 + e8;
    
    /* Additional arithmetic with extracted elements */
    sum = sum * e1 - e2 + e3 * e4 - e5 + e6 * e7 - e8;
#else
    /* Simple fallback */
    sum = i1 + i2 + i3 + i4;
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE static v4sf pattern_d_conditional_vector(
    v4sf a, v4sf b, v4sf c, v4sf d,
    v4sf thresh1, v4sf thresh2,
    int mode1, int mode2)
{
#ifdef __SSE__
    /* Multiple comparisons */
    __m128 cmp1 = _mm_cmpgt_ps(a, thresh1);
    __m128 cmp2 = _mm_cmplt_ps(b, thresh2);
    __m128 cmp3 = _mm_cmpge_ps(c, _mm_set1_ps(0.5f));
    __m128 cmp4 = _mm_cmple_ps(d, _mm_set1_ps(1.5f));
    
    /* Combine masks with logical operations */
    __m128 mask1 = _mm_and_ps(cmp1, cmp2);
    __m128 mask2 = _mm_or_ps(cmp3, cmp4);
    __m128 final_mask = _mm_xor_ps(mask1, mask2);
    
    /* Conditional blending based on complex mask */
    __m128 t1 = _mm_blendv_ps(a, b, final_mask);
    __m128 t2 = _mm_blendv_ps(c, d, final_mask);
    
    /* Final result with arithmetic */
    __m128 result = _mm_add_ps(t1, t2);
    result = _mm_mul_ps(result, final_mask);
    
    return result;
#else
    return a + b + c + d;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int pattern_e_multi_operand_asm(
    int a, int b, int c, int d, int e,
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
        "sub %[r1], %[r1], %[i]\n\t"
        "and %[r1], %[r1], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Vector shuffle with many parameters (x86 specific) */
NOINLINE static v4sf pattern_f_complex_shuffle(
    v4sf v1, v4sf v2, v4sf v3, v4sf v4,
    int imm1, int imm2, int imm3, int imm4)
{
#ifdef __SSE__
    /* Multiple shuffle operations chained */
    __m128 s1 = _mm_shuffle_ps(v1, v2, imm1 & 0xFF);
    __m128 s2 = _mm_shuffle_ps(v3, v4, imm2 & 0xFF);
    __m128 s3 = _mm_shuffle_ps(s1, s2, imm3 & 0xFF);
    __m128 s4 = _mm_shuffle_ps(s2, s1, imm4 & 0xFF);
    
    /* Arithmetic mixing */
    __m128 t1 = _mm_add_ps(s1, s2);
    __m128 t2 = _mm_sub_ps(s3, s4);
    __m128 result = _mm_mul_ps(t1, t2);
    
    /* Additional shuffle */
    result = _mm_shuffle_ps(result, result, (imm1 ^ imm2) & 0xFF);
    
    return result;
#else
    return v1 + v2 + v3 + v4;
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
    v4sf thresh1 = {2.5f, 2.5f, 2.5f, 2.5f};
    v4sf thresh2 = {7.5f, 7.5f, 7.5f, 7.5f};
    
    /* Use argc to select different patterns at runtime */
    int selector = argc > 1 ? atoi(argv[1]) % 6 : 0;
    
    switch (selector) {
        case 0:
            /* Pattern A - Vector blend */
            {
                v4sf result = pattern_a_blend_many_operands(
                    vec1, vec2, vec3, vec4,
                    0x5, 0xA, 0x3, 0xC,
                    1.5f, 2.5f);
                total += ((float*)&result)[0];
            }
            break;
            
        case 1:
            /* Pattern B - FMA chain */
            {
                float result = pattern_b_fma_chain(
                    1.1f, 2.2f, 3.3f,
                    4.4f, 5.5f, 6.6f,
                    7.7f, 8.8f, 9.9f,
                    10.1f, 11.11f);
                total += result;
            }
            break;
            
        case 2:
            /* Pattern C - Vector reduction */
            {
                float result = pattern_c_vector_reduction(
                    vec1, vec2, vec3, vec4,
                    0, 1, 2, 3);
                total += result;
            }
            break;
            
        case 3:
            /* Pattern D - Conditional vector */
            {
                v4sf result = pattern_d_conditional_vector(
                    vec1, vec2, vec3, vec4,
                    thresh1, thresh2,
                    1, 2);
                total += ((float*)&result)[0];
            }
            break;
            
        case 4:
            /* Pattern E - Multi-operand assembly */
            {
                int result = pattern_e_multi_operand_asm(
                    1, 2, 3, 4, 5,
                    6, 7, 8, 9, 10);
                total += result;
            }
            break;
            
        case 5:
            /* Pattern F - Complex shuffle */
            {
                v4sf result = pattern_f_complex_shuffle(
                    vec1, vec2, vec3, vec4,
                    0x1B, 0x27, 0x39, 0x4E);
                total += ((float*)&result)[0];
            }
            break;
    }
    
    /* Ensure result is used to prevent optimization */
    sink = (int)total;
    
    /* Return checksum */
    printf("Result: %f\n", total);
    return (int)total & 0xFF;
}
