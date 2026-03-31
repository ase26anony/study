/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdlib.h>

/* Prevent optimizations from removing our test code */
static volatile int sink = 0;

/* Architecture-specific vector types */
#ifdef __SSE__
#include <xmmintrin.h>
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

#ifdef __SSE2__
#include <emmintrin.h>
typedef double v2df __attribute__((vector_size(16)));
#endif

#ifdef __AVX__
#include <immintrin.h>
typedef float v8sf __attribute__((vector_size(32)));
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
typedef float32x4_t v4sf_neon;
typedef int32x4_t v4si_neon;
#endif

/* Generic fallback types for testing */
typedef float v4sf_generic __attribute__((vector_size(16)));
typedef int v4si_generic __attribute__((vector_size(16)));

/* Pattern A: Complex vector shuffle with many operands */
__attribute__((noinline, noipa))
static v4sf_generic pattern_a_shuffle(v4sf_generic a, v4sf_generic b, 
                                     int mask0, int mask1, int mask2, int mask3,
                                     float scale, float bias) {
    /* Create a complex shuffle pattern that may expand to many operands */
    v4sf_generic temp = __builtin_shuffle(a, b, 
        (v4si_generic){mask0, mask1, mask2, mask3});
    
    /* Additional operations to increase operand count */
    temp = temp * (v4sf_generic){scale, scale, scale, scale};
    temp = temp + (v4sf_generic){bias, bias, bias, bias};
    
    /* Cross-lane operations */
    float sum = temp[0] + temp[1] + temp[2] + temp[3];
    temp[0] = sum;
    
    return temp;
}

/* Pattern B: Fused multiply-add chain */
__attribute__((noinline, noipa))
static float pattern_b_fma_chain(float a, float b, float c, float d,
                                float e, float f, float g, float h,
                                float i, float j, float k) {
    /* Chain of operations that may flatten to many operands */
#ifdef __FMA__
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(t1, t2, t3);
    return __builtin_fma(t4, j, k);
#else
    /* Manual FMA emulation - still creates many operands */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = t1 * t2 + t3;
    return t4 * j + k;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
__attribute__((noinline, noipa))
static float pattern_c_vector_reduce(v4sf_generic v1, v4sf_generic v2,
                                    v4sf_generic v3, v4sf_generic v4) {
    /* Manually extract and process each element */
    float sum = 0.0f;
    
    /* Extract and process each element - each extract is an operand */
    sum += v1[0] * v2[0] + v3[0] * v4[0];
    sum += v1[1] * v2[1] + v3[1] * v4[1];
    sum += v1[2] * v2[2] + v3[2] * v4[2];
    sum += v1[3] * v2[3] + v3[3] * v4[3];
    
    /* Additional operations to increase complexity */
    sum = sum * 2.0f - 1.0f;
    
    return sum;
}

/* Pattern D: Conditional vector operations */
__attribute__((noinline, noipa))
static v4sf_generic pattern_d_conditional_select(v4sf_generic a, v4sf_generic b,
                                                v4sf_generic c, v4sf_generic d,
                                                v4si_generic mask) {
    /* Complex conditional selection that may expand to many operands */
    v4sf_generic temp1 = a * b;
    v4sf_generic temp2 = c * d;
    
    /* Conditional selection based on mask */
    v4sf_generic result;
    for (int i = 0; i < 4; i++) {
        result[i] = (mask[i] & 1) ? temp1[i] : temp2[i];
    }
    
    /* Additional blending */
    result = result + (a + b + c + d) * 0.25f;
    
    return result;
}

/* Pattern E: Inline assembly with many operands */
__attribute__((noinline, noipa))
static void pattern_e_multi_operand_asm(float a, float b, float c, float d,
                                       float e, float f, float g, float h,
                                       float *out1, float *out2, float *out3) {
    /* Inline assembly with exactly 11 operands */
    asm volatile (
        "/* Multi-operand test %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 */"
        : "=m"(*out1), "=m"(*out2), "=m"(*out3)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h)
        : "memory"
    );
}

/* Pattern F: Vector blend with complex mask computation */
#ifdef __SSE4_1__
__attribute__((noinline, noipa))
static __m128 pattern_f_complex_blend(__m128 a, __m128 b, __m128 c, __m128 d,
                                     int mask_bits) {
    /* Complex blend operation that may require many operands */
    __m128 cmp1 = _mm_cmplt_ps(a, b);
    __m128 cmp2 = _mm_cmpgt_ps(c, d);
    __m128 mask = _mm_and_ps(cmp1, cmp2);
    
    /* Additional mask manipulation */
    __m128i mask_int = _mm_castps_si128(mask);
    mask_int = _mm_slli_epi32(mask_int, mask_bits);
    mask = _mm_castsi128_ps(mask_int);
    
    /* Final blend with many operands */
    __m128 t1 = _mm_blendv_ps(a, b, mask);
    __m128 t2 = _mm_blendv_ps(c, d, mask);
    
    return _mm_add_ps(t1, t2);
}
#endif

/* Pattern G: Multi-vector horizontal addition */
__attribute__((noinline, noipa))
static float pattern_g_multi_hadd(v4sf_generic v1, v4sf_generic v2,
                                 v4sf_generic v3, v4sf_generic v4,
                                 v4sf_generic v5, v4sf_generic v6) {
    /* Horizontal addition across multiple vectors */
    float sum = 0.0f;
    
    /* Process each vector - creates many extract operations */
    for (int i = 0; i < 4; i++) {
        sum += v1[i] + v2[i] + v3[i] + v4[i] + v5[i] + v6[i];
    }
    
    /* Additional arithmetic to prevent optimization */
    sum = sum * 0.5f + 1.0f;
    
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    float result = 0.0f;
    
    /* Initialize with some values */
    v4sf_generic v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf_generic v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf_generic v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf_generic v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4si_generic mask = {1, 0, 1, 0};
    
    /* Use argc to select different patterns (ensures all get compiled) */
    switch (argc % 7) {
        case 0:
            result = pattern_a_shuffle(v1, v2, 0, 1, 2, 3, 2.0f, 1.0f)[0];
            break;
        case 1:
            result = pattern_b_fma_chain(1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                                        6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f);
            break;
        case 2:
            result = pattern_c_vector_reduce(v1, v2, v3, v4);
            break;
        case 3: {
            v4sf_generic res = pattern_d_conditional_select(v1, v2, v3, v4, mask);
            result = res[0] + res[1] + res[2] + res[3];
            break;
        }
        case 4: {
            float out1, out2, out3;
            pattern_e_multi_operand_asm(1.0f, 2.0f, 3.0f, 4.0f,
                                       5.0f, 6.0f, 7.0f, 8.0f,
                                       &out1, &out2, &out3);
            result = out1 + out2 + out3;
            break;
        }
        case 5:
#ifdef __SSE4_1__
            {
                __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
                __m128 b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
                __m128 c = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
                __m128 d = _mm_set_ps(13.0f, 14.0f, 15.0f, 16.0f);
                __m128 res = pattern_f_complex_blend(a, b, c, d, 2);
                float temp[4];
                _mm_store_ps(temp, res);
                result = temp[0] + temp[1] + temp[2] + temp[3];
            }
#else
            result = 42.0f;  /* Fallback */
#endif
            break;
        case 6:
            result = pattern_g_multi_hadd(v1, v2, v3, v4, v1, v2);
            break;
    }
    
    /* Use result to prevent optimization */
    sink = (int)result;
    
    /* Return something based on result for validation */
    return (result > 100.0f) ? 1 : 0;
}

/* Additional test functions to ensure various expansion paths */
__attribute__((noinline, noipa))
static void test_many_operands_10(void) {
    /* Direct test with exactly 10 scalar operands in an expression */
    float a = 1.0f, b = 2.0f, c = 3.0f, d = 4.0f, e = 5.0f;
    float f = 6.0f, g = 7.0f, h = 8.0f, i = 9.0f, j = 10.0f;
    
    /* Complex expression that may expand to 10 operands */
    float result = ((a * b) + (c * d) + (e * f) + (g * h) + (i * j)) /
                   (a + b + c + d + e + f + g + h + i + j);
    
    sink = (int)result;
}

__attribute__((noinline, noipa))
static void test_many_operands_11(void) {
    /* Direct test with exactly 11 scalar operands */
    float a = 1.0f, b = 2.0f, c = 3.0f, d = 4.0f, e = 5.0f;
    float f = 6.0f, g = 7.0f, h = 8.0f, i = 9.0f, j = 10.0f, k = 11.0f;
    
    /* Even more complex expression */
    float result = (a * b * c + d * e * f + g * h * i + j * k) /
                   (a + b + c + d + e + f + g + h + i + j + k);
    
    sink = (int)result;
}

/* Force these to be compiled by calling them from a constructor */
__attribute__((constructor))
static void init_tests(void) {
    test_many_operands_10();
    test_many_operands_11();
}
