/* test_optabs_high_operand_count.c
 * 
 * This program is designed to trigger GCC's RTL expansion for operations
 * requiring exactly 10 or 11 operands, covering the switch cases in optabs.cc
 * lines 8254-8263.
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
#endif

/* Prevent optimization and ensure expansion occurs */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent dead code elimination */
static volatile int volatile_sink;

/* Pattern A: Vector blend with complex mask computation (10+ operands) */
NOINLINE static v4sf vector_blend_complex(v4sf a, v4sf b, v4sf c, v4sf d, 
                                          v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE__
    /* Complex blend operation with many operands */
    __m128 v1 = (__m128)a;
    __m128 v2 = (__m128)b;
    __m128 v3 = (__m128)c;
    __m128 v4 = (__m128)d;
    __m128 v5 = (__m128)e;
    __m128 v6 = (__m128)f;
    __m128 v7 = (__m128)g;
    __m128 v8 = (__m128)h;
    
    /* Create complex mask from multiple comparisons */
    __m128 mask1 = _mm_cmplt_ps(v1, v2);  /* 4 comparisons -> multiple operands */
    __m128 mask2 = _mm_cmpgt_ps(v3, v4);
    __m128 mask3 = _mm_cmpeq_ps(v5, v6);
    __m128 mask4 = _mm_cmpneq_ps(v7, v8);
    
    /* Combine masks with logical operations (each adds operands) */
    __m128 combined_mask = _mm_and_ps(mask1, mask2);
    combined_mask = _mm_or_ps(combined_mask, mask3);
    combined_mask = _mm_xor_ps(combined_mask, mask4);
    
    /* Final blend with the combined mask */
    __m128 result = _mm_blendv_ps(v1, v2, combined_mask);
    
    /* Additional arithmetic to ensure expansion */
    result = _mm_add_ps(result, v3);
    result = _mm_sub_ps(result, v4);
    result = _mm_mul_ps(result, v5);
    
    return (v4sf)result;
#else
    /* Fallback for non-SSE targets */
    return a + b + c + d + e + f + g + h;
#endif
}

/* Pattern B: Fused multiply-add chain (deep expression tree) */
NOINLINE static float fma_chain(float a, float b, float c, float d,
                                float e, float f, float g, float h,
                                float i, float j, float k) {
    /* Chain of FMA operations creating deep expression tree */
#ifdef __FP_FAST_FMA
    float res = __builtin_fma(a, b, c);
    res = __builtin_fma(d, e, res);
    res = __builtin_fma(f, g, res);
    res = __builtin_fma(h, i, res);
    res = __builtin_fma(j, k, res);
    
    /* Additional operations to increase operand count */
    res = res * a + b * c - d / e + f * g - h / i + j * k;
    return res;
#else
    /* Manual FMA emulation */
    return a * b + c + d * e + f * g + h * i + j * k;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization (10+ extracts) */
NOINLINE static float vector_reduction_explicit(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    float sum = 0.0f;
    
#ifdef __SSE__
    /* Extract each element individually (each extract adds operands) */
    float v1_0 = ((float*)&v1)[0];
    float v1_1 = ((float*)&v1)[1];
    float v1_2 = ((float*)&v1)[2];
    float v1_3 = ((float*)&v1)[3];
    
    float v2_0 = ((float*)&v2)[0];
    float v2_1 = ((float*)&v2)[1];
    float v2_2 = ((float*)&v2)[2];
    float v2_3 = ((float*)&v2)[3];
    
    float v3_0 = ((float*)&v3)[0];
    float v3_1 = ((float*)&v3)[1];
    float v3_2 = ((float*)&v3)[2];
    float v3_3 = ((float*)&v3)[3];
    
    float v4_0 = ((float*)&v4)[0];
    float v4_1 = ((float*)&v4)[1];
    float v4_2 = ((float*)&v4)[2];
    float v4_3 = ((float*)&v4)[3];
    
    /* Complex reduction with many operands */
    sum = v1_0 + v1_1 + v1_2 + v1_3 +
          v2_0 + v2_1 + v2_2 + v2_3 +
          v3_0 + v3_1 + v3_2 + v3_3 +
          v4_0 + v4_1 + v4_2 + v4_3;
#else
    /* Simple fallback */
    sum = 1.0f + 2.0f + 3.0f + 4.0f;
#endif
    
    return sum;
}

/* Pattern D: Conditional vector move with multi-input mask */
NOINLINE static v4sf conditional_vector_move(v4sf a, v4sf b, v4sf c, v4sf d,
                                             v4sf e, v4sf f, v4sf g, v4sf h) {
#ifdef __SSE__
    /* Multiple comparisons create complex mask */
    __m128 cmp1 = _mm_cmpgt_ps((__m128)a, (__m128)b);
    __m128 cmp2 = _mm_cmplt_ps((__m128)c, (__m128)d);
    __m128 cmp3 = _mm_cmpeq_ps((__m128)e, (__m128)f);
    __m128 cmp4 = _mm_cmpneq_ps((__m128)g, (__m128)h);
    
    /* Combine with logical operations */
    __m128 mask = _mm_and_ps(cmp1, cmp2);
    mask = _mm_or_ps(mask, cmp3);
    mask = _mm_andnot_ps(cmp4, mask);
    
    /* Conditional move based on complex mask */
    __m128 result = _mm_or_ps(
        _mm_and_ps(mask, (__m128)a),
        _mm_andnot_ps(mask, (__m128)b)
    );
    
    /* Additional blending with other vectors */
    result = _mm_blendv_ps(result, (__m128)c, cmp2);
    result = _mm_blendv_ps(result, (__m128)d, cmp3);
    
    return (v4sf)result;
#else
    return a;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int asm_many_operands(int a, int b, int c, int d, int e,
                                      int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline assembly with 11 total operands (1 output + 10 inputs) */
    asm volatile (
        "addl %1, %0\n\t"
        "addl %2, %0\n\t"
        "addl %3, %0\n\t"
        "addl %4, %0\n\t"
        "addl %5, %0\n\t"
        "addl %6, %0\n\t"
        "addl %7, %0\n\t"
        "addl %8, %0\n\t"
        "addl %9, %0\n\t"
        "addl %10, %0"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    return result;
}

/* Pattern F: Complex shuffle with immediate and multiple vectors */
NOINLINE static v4sf complex_shuffle_operation(v4sf a, v4sf b, v4sf c, v4sf d) {
#ifdef __SSE__
    /* Multiple shuffle operations with different masks */
    __m128 shuffled1 = _mm_shuffle_ps((__m128)a, (__m128)b, _MM_SHUFFLE(3, 2, 1, 0));
    __m128 shuffled2 = _mm_shuffle_ps((__m128)c, (__m128)d, _MM_SHUFFLE(0, 1, 2, 3));
    
    /* Blend the results */
    __m128 result = _mm_blend_ps(shuffled1, shuffled2, 0x5);
    
    /* Additional permute */
    result = _mm_permute_ps(result, _MM_SHUFFLE(2, 3, 0, 1));
    
    return (v4sf)result;
#else
    return a;
#endif
}

/* Main test driver */
int main(int argc, char *argv[]) {
    float checksum = 0.0f;
    
    /* Initialize vectors with different values */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf vec5 = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vec6 = {21.0f, 22.0f, 23.0f, 24.0f};
    v4sf vec7 = {25.0f, 26.0f, 27.0f, 28.0f};
    v4sf vec8 = {29.0f, 30.0f, 31.0f, 32.0f};
    
    /* Use argc to select different patterns, ensuring all get compiled */
    switch (argc % 6) {
        case 0:
            /* Pattern A: Vector blend with complex mask */
            {
                v4sf result = vector_blend_complex(vec1, vec2, vec3, vec4,
                                                   vec5, vec6, vec7, vec8);
                checksum += ((float*)&result)[0] + ((float*)&result)[1] +
                           ((float*)&result)[2] + ((float*)&result)[3];
            }
            break;
            
        case 1:
            /* Pattern B: FMA chain with 11 scalar arguments */
            checksum += fma_chain(1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                                  6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f);
            break;
            
        case 2:
            /* Pattern C: Vector reduction with explicit scalarization */
            checksum += vector_reduction_explicit(vec1, vec2, vec3, vec4);
            break;
            
        case 3:
            /* Pattern D: Conditional vector move */
            {
                v4sf result = conditional_vector_move(vec1, vec2, vec3, vec4,
                                                      vec5, vec6, vec7, vec8);
                checksum += ((float*)&result)[0] + ((float*)&result)[3];
            }
            break;
            
        case 4:
            /* Pattern E: Inline assembly with 10 inputs */
            checksum += asm_many_operands(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
            break;
            
        case 5:
            /* Pattern F: Complex shuffle operation */
            {
                v4sf result = complex_shuffle_operation(vec1, vec2, vec3, vec4);
                checksum += ((float*)&result)[0] + ((float*)&result)[2];
            }
            break;
    }
    
    /* Additional test: Mix patterns to ensure multiple expansions */
    if (argc > 1) {
        /* Execute multiple patterns in sequence */
        checksum += fma_chain(checksum, 2.0f, 3.0f, 4.0f, 5.0f,
                              6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f);
        
        v4sf blend_result = vector_blend_complex(vec1, vec2, vec3, vec4,
                                                 vec5, vec6, vec7, vec8);
        checksum += ((float*)&blend_result)[0];
        
        checksum += asm_many_operands((int)checksum, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    }
    
    /* Use volatile sink to prevent optimization */
    volatile_sink = (int)checksum;
    
    printf("Checksum: %f\n", checksum);
    return (checksum > 0.0f) ? 0 : 1;
}
