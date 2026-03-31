/* test_optabs_high_operand.c - Test for covering 10+ operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations from removing our test code */
static volatile int volatile_sink;

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
#include <emmintrin.h>
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
#endif

/* Function to ensure code isn't optimized away */
__attribute__((noinline, noipa))
static int use_result(int val) {
    volatile_sink = val;
    return val;
}

/* Pattern A: Complex vector shuffle with many operands */
__attribute__((noinline, noipa))
static v4sf pattern_a_shuffle_many_ops(v4sf a, v4sf b, v4sf c, v4sf d, 
                                       int imm1, int imm2, int imm3, int imm4) {
#ifdef __SSE__
    /* Create complex shuffle pattern with multiple operations */
    __m128 v1 = _mm_shuffle_ps(a, b, imm1);
    __m128 v2 = _mm_shuffle_ps(c, d, imm2);
    __m128 v3 = _mm_shuffle_ps(v1, v2, imm3);
    __m128 v4 = _mm_shuffle_ps(v2, v1, imm4);
    
    /* Blend operations add more operands */
    __m128 mask = _mm_set_ps(0.0f, 1.0f, 0.0f, 1.0f);
    __m128 result = _mm_blendv_ps(v3, v4, mask);
    
    return result;
#else
    /* Fallback implementation */
    v4sf result = a + b + c + d;
    return result;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
__attribute__((noinline, noipa))
static float pattern_b_fma_chain(float a, float b, float c, float d,
                                 float e, float f, float g, float h,
                                 float i, float j, float k) {
#ifdef __FP_FAST_FMA
    /* Chain of FMA operations - creates deep expression tree */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(t1, t2, t3);
    float result = __builtin_fma(t4, j, k);
    
    /* Additional arithmetic to prevent optimization */
    result = __builtin_fma(result, a, b);
    result = __builtin_fma(result, c, d);
    
    return result;
#else
    /* Manual FMA emulation */
    return ((a * b + c) * (d * e + f) + (g * h + i)) * j + k;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
__attribute__((noinline, noipa))
static float pattern_c_vector_reduction(v4sf vec1, v4sf vec2, v4sf vec3) {
    /* Extract each element manually - creates many extract operations */
    float sum = 0.0f;
    
    /* Extract from first vector */
    sum += ((float*)&vec1)[0];
    sum += ((float*)&vec1)[1];
    sum += ((float*)&vec1)[2];
    sum += ((float*)&vec1)[3];
    
    /* Extract from second vector */
    sum += ((float*)&vec2)[0];
    sum += ((float*)&vec2)[1];
    sum += ((float*)&vec2)[2];
    sum += ((float*)&vec2)[3];
    
    /* Extract from third vector */
    sum += ((float*)&vec3)[0];
    sum += ((float*)&vec3)[1];
    sum += ((float*)&vec3)[2];
    sum += ((float*)&vec3)[3];
    
    /* Additional operations to create more complexity */
    sum = sum * 2.0f - 1.0f;
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
__attribute__((noinline, noipa))
static v4sf pattern_d_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                        v4sf e, v4sf f) {
#ifdef __SSE__
    /* Multiple comparisons create many operands */
    __m128 cmp1 = _mm_cmpgt_ps(a, b);
    __m128 cmp2 = _mm_cmplt_ps(c, d);
    __m128 cmp3 = _mm_cmpeq_ps(e, f);
    
    /* Combine comparison results */
    __m128 mask1 = _mm_and_ps(cmp1, cmp2);
    __m128 mask2 = _mm_or_ps(mask1, cmp3);
    __m128 mask3 = _mm_xor_ps(mask2, cmp1);
    
    /* Multiple blend operations */
    __m128 temp1 = _mm_blendv_ps(a, b, mask1);
    __m128 temp2 = _mm_blendv_ps(c, d, mask2);
    __m128 temp3 = _mm_blendv_ps(e, f, mask3);
    
    /* Final blend with complex mask */
    __m128 final_mask = _mm_and_ps(mask1, _mm_or_ps(mask2, mask3));
    __m128 result = _mm_blendv_ps(temp1, temp2, final_mask);
    result = _mm_blendv_ps(result, temp3, _mm_xor_ps(final_mask, mask3));
    
    return result;
#else
    /* Fallback */
    v4sf result = a + b + c + d + e + f;
    return result;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
__attribute__((noinline, noipa))
static int pattern_e_many_operand_asm(int a, int b, int c, int d, int e,
                                      int f, int g, int h, int i, int j, int k) {
    int result;
    
    /* Inline assembly with 11 operands (10 inputs + 1 output) */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "add %[k], %[a]\n\t"
        "imul %[b], %[c]\n\t"
        "mov %[result], %[a]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result;
}

/* Pattern F: Complex integer vector operations */
__attribute__((noinline, noipa))
static v4si pattern_f_integer_vector_ops(v4si a, v4si b, v4si c, v4si d,
                                         v4si e, v4si f, int imm1, int imm2,
                                         int imm3, int imm4) {
#ifdef __SSE2__
    /* Multiple shuffle operations with different immediates */
    __m128i v1 = _mm_shuffle_epi32(a, imm1);
    __m128i v2 = _mm_shuffle_epi32(b, imm2);
    __m128i v3 = _mm_shuffle_epi32(c, imm3);
    __m128i v4 = _mm_shuffle_epi32(d, imm4);
    
    /* Arithmetic chain */
    __m128i t1 = _mm_add_epi32(v1, v2);
    __m128i t2 = _mm_sub_epi32(v3, v4);
    __m128i t3 = _mm_mullo_epi32(t1, t2);
    __m128i t4 = _mm_add_epi32(e, f);
    
    /* Blend with mask */
    __m128i mask = _mm_set_epi32(0, -1, 0, -1);
    __m128i result = _mm_blendv_epi8(t3, t4, mask);
    
    return result;
#else
    v4si result = a + b + c + d + e + f;
    return result;
#endif
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Initialize test data with some variability based on argc */
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_c = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_d = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf vec_e = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vec_f = {21.0f, 22.0f, 23.0f, 24.0f};
    
    v4si ivec_a = {1, 2, 3, 4};
    v4si ivec_b = {5, 6, 7, 8};
    v4si ivec_c = {9, 10, 11, 12};
    v4si ivec_d = {13, 14, 15, 16};
    v4si ivec_e = {17, 18, 19, 20};
    v4si ivec_f = {21, 22, 23, 24};
    
    /* Use argc to select different patterns, ensuring all get compiled */
    if (argc > 1) {
        /* Pattern A: Complex shuffle with many operands */
        v4sf result_a = pattern_a_shuffle_many_ops(vec_a, vec_b, vec_c, vec_d,
                                                   argc, argc+1, argc+2, argc+3);
        checksum += ((float*)&result_a)[0] + ((float*)&result_a)[1];
    }
    
    if (argc > 2) {
        /* Pattern B: FMA chain - creates deep expression tree */
        float result_b = pattern_b_fma_chain(1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                                            6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 
                                            (float)argc);
        checksum += (int)result_b;
    }
    
    if (argc > 3) {
        /* Pattern C: Vector reduction with explicit scalarization */
        float result_c = pattern_c_vector_reduction(vec_a, vec_b, vec_c);
        checksum += (int)result_c;
    }
    
    if (argc > 4) {
        /* Pattern D: Conditional vector blend with many comparisons */
        v4sf result_d = pattern_d_conditional_blend(vec_a, vec_b, vec_c,
                                                   vec_d, vec_e, vec_f);
        checksum += ((float*)&result_d)[0] + ((float*)&result_d)[1];
    }
    
    if (argc > 5) {
        /* Pattern E: Inline assembly with exactly 11 operands */
        int result_e = pattern_e_many_operand_asm(argc, argc+1, argc+2, argc+3,
                                                 argc+4, argc+5, argc+6, argc+7,
                                                 argc+8, argc+9, argc+10);
        checksum += result_e;
    }
    
    if (argc > 6) {
        /* Pattern F: Integer vector operations with many immediates */
        v4si result_f = pattern_f_integer_vector_ops(ivec_a, ivec_b, ivec_c,
                                                    ivec_d, ivec_e, ivec_f,
                                                    argc, argc+1, argc+2, argc+3);
        checksum += ((int*)&result_f)[0] + ((int*)&result_f)[1];
    }
    
    /* Default case - run all patterns */
    if (argc <= 1) {
        /* Run all patterns to ensure they're all compiled */
        v4sf r1 = pattern_a_shuffle_many_ops(vec_a, vec_b, vec_c, vec_d, 0x1B, 0x4E, 0x93, 0x27);
        checksum += ((float*)&r1)[0];
        
        float r2 = pattern_b_fma_chain(1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 10.5f, 11.5f);
        checksum += (int)r2;
        
        float r3 = pattern_c_vector_reduction(vec_a, vec_b, vec_c);
        checksum += (int)r3;
        
        v4sf r4 = pattern_d_conditional_blend(vec_a, vec_b, vec_c, vec_d, vec_e, vec_f);
        checksum += ((float*)&r4)[0];
        
        int r5 = pattern_e_many_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        checksum += r5;
        
        v4si r6 = pattern_f_integer_vector_ops(ivec_a, ivec_b, ivec_c, ivec_d, ivec_e, ivec_f, 0x1B, 0x4E, 0x93, 0x27);
        checksum += ((int*)&r6)[0];
    }
    
    /* Use the result to prevent dead code elimination */
    return use_result(checksum) % 256;
}
