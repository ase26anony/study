/*
 * test_optabs.c
 * 
 * This program is designed to trigger GCC's RTL expansion for operations
 * requiring exactly 10 or 11 operands, covering the uncovered switch cases
 * in optabs.cc lines 8254-8263.
 *
 * Compile with: gcc -O3 -march=native -fno-tree-vectorize -fprofile-arcs -ftest-coverage -o test_optabs test_optabs.c
 * Run with: ./test_optabs
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical functions */
#define NOOPT __attribute__((noinline, noipa, used))

/* Vector type definitions for various architectures */
#ifdef __SSE__
#include <xmmintrin.h>
typedef __m128 v4sf;
typedef __m128i v4si;
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

#ifdef __AVX__
#include <immintrin.h>
typedef __m256 v8sf;
typedef __m256i v8si;
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
typedef float32x4_t v4sf_neon;
typedef int32x4_t v4si_neon;
#endif

/* Global volatile to prevent dead code elimination */
static volatile float g_volatile_float = 0.0f;
static volatile int g_volatile_int = 0;

/********************** PATTERN A: Vector Blend with Complex Mask **********************/
/* 
 * This pattern uses vector blending with a mask computed from multiple operations.
 * The expansion may generate many operands for the blend operation.
 */
NOOPT v4sf pattern_a_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                                   v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE4_1__
    /* Create complex mask from multiple comparisons */
    v4sf cmp1 = _mm_cmpgt_ps(a, b);
    v4sf cmp2 = _mm_cmplt_ps(c, d);
    v4sf cmp3 = _mm_cmpneq_ps(e, f);
    v4sf cmp4 = _mm_cmpge_ps(g, h);
    
    /* Combine masks with logical operations */
    v4sf mask1 = _mm_and_ps(cmp1, cmp2);
    v4sf mask2 = _mm_or_ps(cmp3, cmp4);
    v4sf final_mask = _mm_xor_ps(mask1, mask2);
    
    /* Use blendv with the complex mask */
    return _mm_blendv_ps(a, b, final_mask);
#else
    /* Fallback implementation */
    v4sf result = a + b + c + d + e + f + g + h;
    return result;
#endif
}

/********************** PATTERN B: Fused Multiply-Add Chain **********************/
/*
 * Chain of FMA operations creating a deep expression tree.
 * During RTL expansion, this may be flattened into many operands.
 */
NOOPT float pattern_b_fma_chain(float a, float b, float c, float d,
                                float e, float f, float g, float h,
                                float i, float j, float k, float l) {
#ifdef __FMA__
    /* Chain of FMA operations - each adds 3 operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    
    /* Combine results with more FMAs */
    float t5 = __builtin_fmaf(t1, t2, t3);
    float t6 = __builtin_fmaf(t4, t5, t1);
    
    return __builtin_fmaf(t6, t5, t4);
#else
    /* Manual FMA simulation */
    return a*b + c + d*e + f + g*h + i + j*k + l;
#endif
}

/********************** PATTERN C: Vector Reduction with Explicit Scalarization **********************/
/*
 * Manually unrolled vector horizontal addition with lane extraction.
 * Each extract operation adds operands during expansion.
 */
NOOPT float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    float sum = 0.0f;
    
#ifdef __SSE__
    /* Extract each lane explicitly - creates many extract operations */
    float v1_0 = _mm_cvtss_f32(v1);
    float v1_1 = _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 1, 1, 1)));
    float v1_2 = _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(2, 2, 2, 2)));
    float v1_3 = _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 3, 3, 3)));
    
    float v2_0 = _mm_cvtss_f32(v2);
    float v2_1 = _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(1, 1, 1, 1)));
    float v2_2 = _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(2, 2, 2, 2)));
    float v2_3 = _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 3, 3, 3)));
    
    /* Additional operations to increase operand count */
    sum += v1_0 * v2_0 + v1_1 * v2_1 + v1_2 * v2_2 + v1_3 * v2_3;
    
    /* More extractions from v3 and v4 */
    float v3_0 = _mm_cvtss_f32(v3);
    float v3_1 = _mm_cvtss_f32(_mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 1, 1, 1)));
    float v4_0 = _mm_cvtss_f32(v4);
    float v4_1 = _mm_cvtss_f32(_mm_shuffle_ps(v4, v4, _MM_SHUFFLE(1, 1, 1, 1)));
    
    sum += v3_0 * v4_0 + v3_1 * v4_1;
#else
    /* Fallback */
    float* f1 = (float*)&v1;
    float* f2 = (float*)&v2;
    sum = f1[0] + f1[1] + f1[2] + f1[3] + f2[0] + f2[1] + f2[2] + f2[3];
#endif
    
    return sum;
}

/********************** PATTERN D: Conditional Vector Move with Multi-Input Mask **********************/
/*
 * Complex conditional select using multiple vector comparisons and logical ops.
 */
NOOPT v4sf pattern_d_conditional_move(v4sf a, v4sf b, v4sf c, v4sf d,
                                      v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE__
    /* Multiple comparisons */
    v4sf cmp1 = _mm_cmpgt_ps(a, b);
    v4sf cmp2 = _mm_cmplt_ps(c, d);
    v4sf cmp3 = _mm_cmpneq_ps(e, f);
    v4sf cmp4 = _mm_cmpge_ps(g, h);
    
    /* Complex mask computation with many operations */
    v4sf mask_a = _mm_and_ps(cmp1, cmp2);
    v4sf mask_b = _mm_or_ps(cmp3, cmp4);
    v4sf mask_c = _mm_xor_ps(mask_a, mask_b);
    v4sf mask_d = _mm_andnot_ps(mask_c, cmp1);
    
    /* Blend based on complex mask */
    v4sf temp1 = _mm_and_ps(a, mask_d);
    v4sf temp2 = _mm_andnot_ps(mask_d, b);
    v4sf result = _mm_or_ps(temp1, temp2);
    
    /* Additional operations to ensure expansion */
    result = _mm_add_ps(result, _mm_and_ps(c, mask_c));
    result = _mm_sub_ps(result, _mm_andnot_ps(mask_b, d));
    
    return result;
#else
    return a + b + c + d + e + f + g + h;
#endif
}

/********************** PATTERN E: Inline Assembly with Many Operands **********************/
/*
 * Direct inline assembly with exactly 10-11 operands.
 * This should directly create an RTL insn with the required operand count.
 */
NOOPT int64_t pattern_e_multi_operand_asm(int64_t a, int64_t b, int64_t c, 
                                         int64_t d, int64_t e, int64_t f,
                                         int64_t g, int64_t h, int64_t i,
                                         int64_t j) {
    int64_t result1, result2;
    
    /* Inline assembly with 10 explicit input/output operands */
    __asm__ volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "imul %[i], %[j]\n\t"
        : [a] "+r" (a), [b] "+r" (b), [c] "+r" (c), [d] "+r" (d),
          [e] "+r" (e), [f] "+r" (f), [g] "+r" (g), [h] "+r" (h),
          [result1] "=r" (result1), [result2] "=r" (result2)
        : [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2 + a + b + c + d + e + f + g + h + i + j;
}

/********************** PATTERN F: Vector Shuffle with Many Operands **********************/
/*
 * Vector shuffle operations with complex immediate computation.
 * The shuffle mask may be computed from multiple operands.
 */
NOOPT v4sf pattern_f_complex_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                                     int mask1, int mask2, int mask3, int mask4) {
#ifdef __SSE__
    /* Multiple shuffle operations with different masks */
    v4sf s1 = _mm_shuffle_ps(a, b, mask1);
    v4sf s2 = _mm_shuffle_ps(c, d, mask2);
    v4sf s3 = _mm_shuffle_ps(s1, s2, mask3);
    v4sf s4 = _mm_shuffle_ps(s2, s1, mask4);
    
    /* Combine results */
    v4sf result = _mm_add_ps(s3, s4);
    result = _mm_sub_ps(result, _mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 3, 0, 1)));
    
    return result;
#else
    return a + b + c + d;
#endif
}

/********************** Main Test Function **********************/
int main(int argc, char** argv) {
    float checksum = 0.0f;
    
    /* Initialize test data with some variability based on argc */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf vec5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vec6 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf vec7 = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf vec8 = {29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Use argc to select different patterns, ensuring all get compiled */
    if (argc > 1) {
        /* Pattern A: Complex blend */
        v4sf res_a = pattern_a_blend_complex(vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8);
        checksum += ((float*)&res_a)[0] + ((float*)&res_a)[1];
        
        /* Pattern B: FMA chain */
        float res_b = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f,
                                         7.7f, 8.8f, 9.9f, 10.1f, 11.11f, 12.12f);
        checksum += res_b;
    }
    
    if (argc > 2) {
        /* Pattern C: Vector reduction */
        float res_c = pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
        checksum += res_c;
        
        /* Pattern D: Conditional move */
        v4sf res_d = pattern_d_conditional_move(vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8);
        checksum += ((float*)&res_d)[2] + ((float*)&res_d)[3];
    }
    
    if (argc > 3) {
        /* Pattern E: Multi-operand assembly */
        int64_t res_e = pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        checksum += (float)res_e;
        
        /* Pattern F: Complex shuffle */
        v4sf res_f = pattern_f_complex_shuffle(vec1, vec2, vec3, vec4,
                                              _MM_SHUFFLE(0, 1, 2, 3),
                                              _MM_SHUFFLE(3, 2, 1, 0),
                                              _MM_SHUFFLE(1, 0, 3, 2),
                                              _MM_SHUFFLE(2, 3, 0, 1));
        checksum += ((float*)&res_f)[0] + ((float*)&res_f)[1] + ((float*)&res_f)[2] + ((float*)&res_f)[3];
    }
    
    /* Store to volatile to prevent optimization */
    g_volatile_float = checksum;
    g_volatile_int = (int)checksum;
    
    printf("Checksum: %f\n", checksum);
    return (int)checksum % 256;
}
