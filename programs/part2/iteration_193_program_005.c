/* test_optabs_coverage.c
 * 
 * This test program is designed to trigger the 10 and 11 operand switch cases
 * in optabs.cc during RTL expansion. It uses various patterns that require
 * many operands during expansion, particularly targeting SIMD operations
 * and complex arithmetic expressions.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical functions */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
#endif

/* Global volatile to prevent dead code elimination */
volatile int g_volatile_result = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE v4sf pattern_a_shuffle_many_operands(v4sf a, v4sf b, v4sf c, v4sf d,
                                              int mask1, int mask2, int mask3) {
#ifdef __SSE__
    /* This creates many operands: 4 vectors + 3 masks + temporaries */
    __m128 t1 = _mm_shuffle_ps(a, b, mask1);
    __m128 t2 = _mm_shuffle_ps(c, d, mask2);
    __m128 t3 = _mm_shuffle_ps(t1, t2, mask3);
    
    /* Additional operations to increase operand count */
    __m128 t4 = _mm_add_ps(t1, t2);
    __m128 t5 = _mm_mul_ps(t3, t4);
    __m128 t6 = _mm_sub_ps(t5, a);
    
    return t6;
#else
    /* Fallback implementation */
    v4sf t1 = __builtin_shuffle(a, b, (v4si){0,1,2,3});
    v4sf t2 = __builtin_shuffle(c, d, (v4si){0,1,2,3});
    v4sf t3 = __builtin_shuffle(t1, t2, (v4si){0,1,2,3});
    return t3 + t1 - a;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE double pattern_b_fma_chain(double a, double b, double c, 
                                    double d, double e, double f,
                                    double g, double h, double i, double j) {
    /* This creates a complex expression tree that may be flattened
     * into many operands during expansion */
#ifdef __FMA__
    double t1 = __builtin_fma(a, b, c);
    double t2 = __builtin_fma(d, e, f);
    double t3 = __builtin_fma(g, h, i);
    double t4 = __builtin_fma(t1, t2, t3);
    return __builtin_fma(t4, j, a + b + c + d);
#else
    /* Manual FMA emulation to still create many operands */
    double t1 = a * b + c;
    double t2 = d * e + f;
    double t3 = g * h + i;
    double t4 = t1 * t2 + t3;
    return t4 * j + (a + b + c + d);
#endif
}

/* Pattern C: Vector extraction and horizontal sum with many operands */
NOINLINE float pattern_c_vector_extract_sum(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Extracting each element creates separate operations */
#ifdef __SSE__
    float sum = 0.0f;
    
    /* Extract all elements - each extract is an operation */
    sum += _mm_cvtss_f32(v1);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 3, 3, 3)));
    
    sum += _mm_cvtss_f32(v2);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 3, 3, 3)));
    
    /* Mix with other vectors */
    v4sf t1 = _mm_add_ps(v1, v2);
    v4sf t2 = _mm_add_ps(v3, v4);
    v4sf t3 = _mm_mul_ps(t1, t2);
    
    sum += _mm_cvtss_f32(t3);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(t3, t3, _MM_SHUFFLE(1, 1, 1, 1)));
    
    return sum;
#else
    /* Fallback with many scalar operations */
    float* f1 = (float*)&v1;
    float* f2 = (float*)&v2;
    float* f3 = (float*)&v3;
    float* f4 = (float*)&v4;
    
    return f1[0] + f1[1] + f1[2] + f1[3] +
           f2[0] + f2[1] + f2[2] + f2[3] +
           f3[0] + f3[1] + f3[2] + f3[3] +
           f4[0] + f4[1] + f4[2] + f4[3];
#endif
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE v4sf pattern_d_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                          v4sf mask1, v4sf mask2) {
#ifdef __SSE__
    /* Multiple comparisons and blends create many operands */
    __m128 cmp1 = _mm_cmpgt_ps(a, b);
    __m128 cmp2 = _mm_cmplt_ps(c, d);
    __m128 cmp3 = _mm_cmpeq_ps(mask1, mask2);
    
    __m128 t1 = _mm_and_ps(cmp1, a);
    __m128 t2 = _mm_andnot_ps(cmp1, b);
    __m128 blend1 = _mm_or_ps(t1, t2);
    
    __m128 t3 = _mm_and_ps(cmp2, c);
    __m128 t4 = _mm_andnot_ps(cmp2, d);
    __m128 blend2 = _mm_or_ps(t3, t4);
    
    __m128 t5 = _mm_and_ps(cmp3, blend1);
    __m128 t6 = _mm_andnot_ps(cmp3, blend2);
    
    return _mm_or_ps(t5, t6);
#else
    /* Manual blend */
    v4sf cmp1 = a > b;
    v4sf cmp2 = c < d;
    v4sf cmp3 = mask1 == mask2;
    
    v4sf blend1 = cmp1 ? a : b;
    v4sf blend2 = cmp2 ? c : d;
    return cmp3 ? blend1 : blend2;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE int64_t pattern_e_multi_operand_asm(int64_t a, int64_t b, int64_t c,
                                            int64_t d, int64_t e, int64_t f,
                                            int64_t g, int64_t h, int64_t i,
                                            int64_t j, int64_t k) {
    int64_t result1, result2, result3;
    
    /* This inline asm has 11 operands total (10 inputs + 1 output) */
    asm volatile (
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[b]\n\t"
        "add %[r1], %[c]\n\t"
        "mov %[r2], %[d]\n\t"
        "add %[r2], %[e]\n\t"
        "add %[r2], %[f]\n\t"
        "mov %[r3], %[g]\n\t"
        "add %[r3], %[h]\n\t"
        "add %[r3], %[i]\n\t"
        "imul %[r1], %[r2]\n\t"
        "add %[r1], %[r3]\n\t"
        "add %[r1], %[j]\n\t"
        "add %[r1], %[k]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1;
}

/* Pattern F: Complex bit manipulation with many operands */
NOINLINE uint64_t pattern_f_bit_ops(uint64_t a, uint64_t b, uint64_t c,
                                   uint64_t d, uint64_t e, uint64_t f,
                                   uint64_t g, uint64_t h, uint64_t i,
                                   uint64_t j) {
    /* This creates a complex expression with many operands */
    uint64_t t1 = (a & b) | (c & d);
    uint64_t t2 = (e ^ f) & (g | h);
    uint64_t t3 = (i << 5) | (j >> 3);
    uint64_t t4 = (t1 ^ t2) & ~t3;
    uint64_t t5 = (a | c | e | g | i) & (b | d | f | h | j);
    
    return (t4 + t5) ^ (a + b + c + d + e + f + g + h + i + j);
}

/* Main test driver */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Use argc to add runtime variability and ensure all paths are compiled */
    int test_case = argc > 1 ? atoi(argv[1]) % 6 : 0;
    
    /* Initialize test data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Execute different patterns based on test_case */
    switch (test_case) {
        case 0:
            /* Pattern A: Vector shuffle with many operands */
            {
                v4sf res = pattern_a_shuffle_many_operands(vec1, vec2, vec3, vec4,
                                                          0x1B, 0x4E, 0x93);
                float* f = (float*)&res;
                result = (int)(f[0] + f[1] + f[2] + f[3]);
            }
            break;
            
        case 1:
            /* Pattern B: FMA chain */
            {
                double res = pattern_b_fma_chain(1.0, 2.0, 3.0, 4.0, 5.0, 6.0,
                                                7.0, 8.0, 9.0, 10.0);
                result = (int)res;
            }
            break;
            
        case 2:
            /* Pattern C: Vector extract sum */
            {
                float res = pattern_c_vector_extract_sum(vec1, vec2, vec3, vec4);
                result = (int)res;
            }
            break;
            
        case 3:
            /* Pattern D: Conditional blend */
            {
                v4sf mask1 = {1.0f, 0.0f, 1.0f, 0.0f};
                v4sf mask2 = {0.0f, 1.0f, 0.0f, 1.0f};
                v4sf res = pattern_d_conditional_blend(vec1, vec2, vec3, vec4,
                                                      mask1, mask2);
                float* f = (float*)&res;
                result = (int)(f[0] + f[1] + f[2] + f[3]);
            }
            break;
            
        case 4:
            /* Pattern E: Multi-operand inline assembly */
            {
                int64_t res = pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6,
                                                        7, 8, 9, 10, 11);
                result = (int)res;
            }
            break;
            
        case 5:
            /* Pattern F: Bit operations */
            {
                uint64_t res = pattern_f_bit_ops(0x12345678, 0x87654321,
                                                0xABCDEF01, 0xFEDCBA09,
                                                0x13579BDF, 0x2468ACE0,
                                                0x11223344, 0x55667788,
                                                0x99AABBCC, 0xDDEEFF00);
                result = (int)res;
            }
            break;
    }
    
    /* Store result in volatile to prevent optimization */
    g_volatile_result = result;
    
    /* Also use result to affect return value */
    return result % 256;
}

/* Additional test to ensure all patterns are compiled */
void compile_all_patterns() {
    /* Force compilation of all patterns by calling them */
    v4sf v1 = {0}, v2 = {0}, v3 = {0}, v4 = {0};
    v4sf mask1 = {0}, mask2 = {0};
    
    pattern_a_shuffle_many_operands(v1, v2, v3, v4, 0, 0, 0);
    pattern_b_fma_chain(0,0,0,0,0,0,0,0,0,0);
    pattern_c_vector_extract_sum(v1, v2, v3, v4);
    pattern_d_conditional_blend(v1, v2, v3, v4, mask1, mask2);
    pattern_e_multi_operand_asm(0,0,0,0,0,0,0,0,0,0,0);
    pattern_f_bit_ops(0,0,0,0,0,0,0,0,0,0);
}
