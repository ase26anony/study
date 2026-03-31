/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent aggressive optimization */
static volatile int sink;

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
#define HAS_SSE 1
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#elif defined(__arm__) || defined(__aarch64__)
#define HAS_NEON 1
#include <arm_neon.h>
#else
#define HAS_GENERIC 1
#endif

/* Function to ensure code isn't optimized away */
__attribute__((noinline, noipa))
static int use_result(float f) {
    sink = (int)f;
    return sink;
}

/* Pattern A: Complex vector shuffle with many operands */
__attribute__((noinline, noipa))
#ifdef HAS_SSE
static v4sf pattern_a_shuffle(v4sf a, v4sf b, int mask1, int mask2, 
                              float f1, float f2, float f3, float f4) {
    /* Create multiple shuffle operations that may expand to many operands */
    v4sf t1 = _mm_shuffle_ps(a, b, mask1);
    v4sf t2 = _mm_shuffle_ps(b, a, mask2);
    
    /* Blend with immediate - may generate many operands during expansion */
    v4sf result = _mm_blend_ps(t1, t2, 0x5);  /* 0101 pattern */
    
    /* Additional arithmetic to prevent elimination */
    v4sf scale = {f1, f2, f3, f4};
    return result * scale;
}
#endif

/* Pattern B: FMA chain creating deep expression tree */
__attribute__((noinline, noipa))
#ifdef __FMA__
static float pattern_b_fma_chain(float a, float b, float c, float d,
                                 float e, float f, float g, float h,
                                 float i, float j, float k, float l) {
    /* Chain of FMA operations - may flatten to many operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    
    /* Combine results with more arithmetic */
    return __builtin_fmaf(t1, t2, __builtin_fmaf(t3, t4, 0.0f));
}
#endif

/* Pattern C: Vector reduction with explicit scalarization */
__attribute__((noinline, noipa))
#ifdef HAS_SSE
static float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Extract each element manually - creates many extract operations */
    float sum = 0.0f;
    
    /* Force extraction of each element */
    sum += v1[0] + v1[1] + v1[2] + v1[3];
    sum += v2[0] + v2[1] + v2[2] + v2[3];
    sum += v3[0] + v3[1] + v3[2] + v3[3];
    sum += v4[0] + v4[1] + v4[2] + v4[3];
    
    /* Additional operations to increase operand count */
    v4sf temp = v1 + v2;
    sum += temp[0] * temp[1] - temp[2] / (temp[3] + 1.0f);
    
    return sum;
}
#endif

/* Pattern D: Conditional vector operations */
__attribute__((noinline, noipa))
#ifdef HAS_SSE
static v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                         v4sf mask1, v4sf mask2) {
    /* Multiple comparisons and blends */
    v4sf cmp1 = _mm_cmpgt_ps(a, b);
    v4sf cmp2 = _mm_cmplt_ps(c, d);
    
    /* Combine masks with logical operations */
    v4sf combined_mask = _mm_and_ps(cmp1, cmp2);
    v4sf not_mask = _mm_andnot_ps(combined_mask, mask1);
    
    /* Multiple blend operations */
    v4sf t1 = _mm_blendv_ps(a, b, combined_mask);
    v4sf t2 = _mm_blendv_ps(c, d, not_mask);
    
    /* Final operation with many operands */
    return _mm_add_ps(_mm_mul_ps(t1, t2), _mm_sub_ps(a, b));
}
#endif

/* Pattern E: Inline assembly with many operands */
__attribute__((noinline, noipa))
static int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                       int f, int g, int h, int i, int j) {
    int result1, result2, result3;
    
    /* Inline assembly with exactly 10 explicit operands */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r2], %[d], %[e]\n\t"
        "add %[r2], %[r2], %[f]\n\t"
        "add %[r3], %[g], %[h]\n\t"
        "add %[r3], %[r3], %[i]\n\t"
        "mul %[out], %[r1], %[r2]\n\t"
        "add %[out], %[out], %[r3]\n\t"
        "add %[out], %[out], %[j]"
        : [out] "=r" (result1), [r1] "=&r" (result2), [r2] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    return result1;
}

/* Pattern F: AVX2 gather operation (many operands) */
__attribute__((noinline, noipa))
#ifdef __AVX2__
static v8sf pattern_f_gather_operation(v8si indices, float* base,
                                       v8sf src, v8sf mask) {
    /* AVX2 gather with scale - may expand to many operands */
    return _mm256_mask_i32gather_ps(src, base, indices, mask, 4);
}
#endif

/* Main test driver */
int main(int argc, char** argv) {
    float result = 0.0f;
    
    /* Use argc to add runtime variability */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize test data */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf v4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    /* Test different patterns based on architecture */
#if defined(HAS_SSE)
    /* Pattern A */
    v4sf a_result = pattern_a_shuffle(v1, v2, 0x1B, 0x27,
                                      rand() % 100 / 100.0f,
                                      rand() % 100 / 100.0f,
                                      rand() % 100 / 100.0f,
                                      rand() % 100 / 100.0f);
    result += use_result(a_result[0]);
    
    /* Pattern C */
    float c_result = pattern_c_vector_reduce(v1, v2, v3, v4);
    result += use_result(c_result);
    
    /* Pattern D */
    v4sf d_result = pattern_d_conditional_select(v1, v2, v3, v4, v1, v2);
    result += use_result(d_result[0]);
#endif
    
#if defined(__FMA__)
    /* Pattern B */
    float b_result = pattern_b_fma_chain(
        1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
        7.7f, 8.8f, 9.9f, 10.1f, 11.1f, 12.1f);
    result += use_result(b_result);
#endif
    
    /* Pattern E (always available) */
    int e_result = pattern_e_multi_operand_asm(
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    result += use_result((float)e_result);
    
#if defined(__AVX2__)
    /* Pattern F */
    v8si indices = {0, 4, 8, 12, 16, 20, 24, 28};
    float base_array[32];
    for (int i = 0; i < 32; i++) base_array[i] = i * 1.0f;
    v8sf src = {0.0f};
    v8sf mask = {1.0f};
    
    v8sf f_result = pattern_f_gather_operation(indices, base_array, src, mask);
    result += use_result(f_result[0]);
#endif
    
    /* Return checksum to prevent dead code elimination */
    printf("Result checksum: %f\n", result);
    return (int)result % 256;
}

/* Fallback implementations for architectures without SIMD */
#ifdef HAS_GENERIC
/* Dummy implementations that still generate some code */
__attribute__((noinline, noipa))
static float generic_pattern(float a, float b, float c, float d,
                             float e, float f, float g, float h,
                             float i, float j, float k, float l) {
    /* Complex expression that may use many operands */
    return (((a * b + c) * d - e) / f + g) * h - i / j + k * l;
}

__attribute__((noinline, noipa))
static int generic_multi_op(int a, int b, int c, int d, int e,
                            int f, int g, int h, int i, int j) {
    /* Chain of operations */
    int t1 = a + b * c;
    int t2 = d - e / f;
    int t3 = g ^ h | i;
    return t1 * t2 + t3 - j;
}
#endif
