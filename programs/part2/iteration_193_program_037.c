/* test_optabs_high_operand_count.c
 * 
 * This test targets GCC's optabs.cc expansion for operations with 10-11 operands.
 * It uses various patterns to trigger the specific switch cases at lines 8254-8263.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline, noipa, used))
#define VOLATILE_USE(x) do { volatile int __v = (int)(x); (void)__v; } while(0)

/* Vector type definitions for portability */
#ifdef __SSE__
#include <xmmintrin.h>
#include <emmintrin.h>
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
#endif

/* Pattern A: Complex vector blend with many operands */
NOINLINE v4sf pattern_a_blend_many_operands(v4sf a, v4sf b, v4sf c, v4sf d, 
                                           v4sf e, v4sf f, v4sf g, v4sf h,
                                           int mask1, int mask2, int mask3) {
    /* This should expand to many operands during RTL generation */
#ifdef __SSE4_1__
    /* Use blendps with dynamic mask computation - may expand to many operands */
    __m128 v1 = _mm_blend_ps((__m128)a, (__m128)b, mask1);
    __m128 v2 = _mm_blend_ps((__m128)c, (__m128)d, mask2);
    __m128 v3 = _mm_blend_ps((__m128)e, (__m128)f, mask3);
    __m128 v4 = _mm_blend_ps((__m128)g, (__m128)h, mask1 ^ mask2);
    
    /* Chain operations to increase operand count */
    v1 = _mm_add_ps(v1, v2);
    v3 = _mm_add_ps(v3, v4);
    return (v4sf)_mm_add_ps(v1, v3);
#else
    /* Fallback implementation */
    v4sf v1 = a + b;
    v4sf v2 = c + d;
    v4sf v3 = e + f;
    v4sf v4 = g + h;
    return v1 + v2 + v3 + v4;
#endif
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE float pattern_b_fma_chain(float a, float b, float c, float d, float e,
                                  float f, float g, float h, float i, float j) {
    /* This creates a complex expression tree that may be flattened to many operands */
#ifdef __FMA__
    /* Use __builtin_fmaf to create dependency chain */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(t1, t2, t3);
    return __builtin_fmaf(t4, j, a + b + c + d);
#else
    /* Manual FMA emulation - still creates many operands */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = t1 * t2 + t3;
    return t4 * j + (a + b + c + d);
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Extract each element manually to create many operands */
    float sum = 0.0f;
    
    /* Force extraction of each element - each extract is an operand */
#ifdef __SSE__
    sum += ((float*)&v1)[0] + ((float*)&v1)[1] + ((float*)&v1)[2] + ((float*)&v1)[3];
    sum += ((float*)&v2)[0] + ((float*)&v2)[1] + ((float*)&v2)[2] + ((float*)&v2)[3];
    sum += ((float*)&v3)[0] + ((float*)&v3)[1] + ((float*)&v3)[2] + ((float*)&v3)[3];
    sum += ((float*)&v4)[0] + ((float*)&v4)[1] + ((float*)&v4)[2] + ((float*)&v4)[3];
#else
    /* Portable version */
    for (int i = 0; i < 4; i++) sum += v1[i] + v2[i] + v3[i] + v4[i];
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE v4sf pattern_d_conditional_vector(v4sf a, v4sf b, v4sf c, v4sf d,
                                          v4sf e, v4sf f, v4sf g, v4sf h) {
    /* Multiple comparisons and blends create many operands */
#ifdef __SSE__
    __m128 cmp1 = _mm_cmpgt_ps((__m128)a, (__m128)b);
    __m128 cmp2 = _mm_cmpgt_ps((__m128)c, (__m128)d);
    __m128 cmp3 = _mm_cmpgt_ps((__m128)e, (__m128)f);
    __m128 cmp4 = _mm_cmpgt_ps((__m128)g, (__m128)h);
    
    /* Combine comparisons - each operation adds operands */
    __m128 mask1 = _mm_and_ps(cmp1, cmp2);
    __m128 mask2 = _mm_and_ps(cmp3, cmp4);
    __m128 final_mask = _mm_or_ps(mask1, mask2);
    
    /* Select based on mask */
    __m128 sel1 = _mm_blendv_ps((__m128)a, (__m128)b, cmp1);
    __m128 sel2 = _mm_blendv_ps((__m128)c, (__m128)d, cmp2);
    __m128 result = _mm_add_ps(sel1, sel2);
    
    __m128 sel3 = _mm_blendv_ps((__m128)e, (__m128)f, cmp3);
    __m128 sel4 = _mm_blendv_ps((__m128)g, (__m128)h, cmp4);
    result = _mm_add_ps(result, _mm_add_ps(sel3, sel4));
    
    /* Final blend with mask */
    return (v4sf)_mm_blendv_ps(result, (__m128)a, final_mask);
#else
    /* Portable fallback */
    v4sf result = a + b + c + d + e + f + g + h;
    return result;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE int64_t pattern_e_asm_many_operands(int64_t a, int64_t b, int64_t c, 
                                            int64_t d, int64_t e, int64_t f,
                                            int64_t g, int64_t h, int64_t i,
                                            int64_t j, int64_t k) {
    int64_t result1, result2, result3;
    
    /* Inline assembly with 11 explicit operands (10 inputs + 1 output) */
    asm volatile (
        /* Complex operation chain using all inputs */
        "addq %[a], %[b]\n\t"
        "addq %[c], %[d]\n\t"
        "addq %[e], %[f]\n\t"
        "addq %[g], %[h]\n\t"
        "imulq %[i], %[j]\n\t"
        "addq %[b], %[d]\n\t"
        "addq %[f], %[h]\n\t"
        "addq %[d], %[h]\n\t"
        "addq %[h], %[j]\n\t"
        "addq %[k], %[j]\n\t"
        "movq %[j], %[out1]\n\t"
        
        /* Additional operations to ensure expansion */
        "movq %[out1], %[out2]\n\t"
        "addq $1, %[out2]\n\t"
        "movq %[out2], %[out3]"
        
        : [out1] "=r" (result1), [out2] "=r" (result2), [out3] "=r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Pattern F: Shuffle with complex immediate computation */
NOINLINE v4sf pattern_f_complex_shuffle(v4sf a, v4sf b, v4sf c, v4sf d,
                                       int imm1, int imm2, int imm3, int imm4) {
    /* Complex shuffle pattern that may expand to many operands */
#ifdef __SSE__
    /* Multiple shuffles with computed immediates */
    __m128 s1 = _mm_shuffle_ps((__m128)a, (__m128)b, imm1);
    __m128 s2 = _mm_shuffle_ps((__m128)c, (__m128)d, imm2);
    
    /* Blend the results */
    __m128 b1 = _mm_blend_ps(s1, s2, imm3);
    
    /* Another shuffle on blended result */
    __m128 s3 = _mm_shuffle_ps(b1, (__m128)a, imm4);
    
    /* Final blend */
    return (v4sf)_mm_blend_ps(b1, s3, imm1 ^ imm2);
#else
    /* Portable fallback */
    v4sf s1 = a + b;
    v4sf s2 = c + d;
    return s1 + s2;
#endif
}

/* Main test driver */
int main(int argc, char *argv[]) {
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
    
    int mask1 = argc > 1 ? atoi(argv[1]) : 0xF;
    int mask2 = argc > 2 ? atoi(argv[2]) : 0xA;
    int mask3 = argc > 3 ? atoi(argv[3]) : 0x5;
    int mask4 = argc > 4 ? atoi(argv[4]) : 0x3;
    
    /* Execute all patterns to ensure compilation */
    
    /* Pattern A: 11 arguments (8 vectors + 3 masks) */
    v4sf result_a = pattern_a_blend_many_operands(vec1, vec2, vec3, vec4,
                                                 vec5, vec6, vec7, vec8,
                                                 mask1, mask2, mask3);
    VOLATILE_USE(result_a[0] + result_a[1] + result_a[2] + result_a[3]);
    
    /* Pattern B: 10 float arguments */
    float result_b = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f,
                                        6.6f, 7.7f, 8.8f, 9.9f, 10.10f);
    VOLATILE_USE(result_b);
    
    /* Pattern C: 4 vector arguments (but many extracted operands) */
    float result_c = pattern_c_vector_reduction(vec1, vec2, vec3, vec4);
    VOLATILE_USE(result_c);
    
    /* Pattern D: 8 vector arguments */
    v4sf result_d = pattern_d_conditional_vector(vec1, vec2, vec3, vec4,
                                                vec5, vec6, vec7, vec8);
    VOLATILE_USE(result_d[0] + result_d[1] + result_d[2] + result_d[3]);
    
    /* Pattern E: 11 integer arguments */
    int64_t result_e = pattern_e_asm_many_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    VOLATILE_USE(result_e);
    
    /* Pattern F: 8 arguments (4 vectors + 4 immediates) */
    v4sf result_f = pattern_f_complex_shuffle(vec1, vec2, vec3, vec4,
                                             mask1, mask2, mask3, mask4);
    VOLATILE_USE(result_f[0] + result_f[1] + result_f[2] + result_f[3]);
    
    /* Combine all results into final checksum */
    checksum = result_b + result_c + (float)result_e;
    checksum += result_a[0] + result_d[0] + result_f[0];
    
    printf("Test completed with checksum: %f\n", checksum);
    
    /* Return non-zero if checksum is suspiciously small */
    return (checksum < 100.0f) ? 1 : 0;
}
