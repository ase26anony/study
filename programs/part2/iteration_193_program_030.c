/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Define vector types for portability */
#if defined(__SSE__) || defined(__AVX__)
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#else
/* Fallback definitions */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE static v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                                       int mask1, int mask2, int mask3, int mask4) {
#if defined(__SSE__)
    /* Multiple shuffle operations that may expand to many operands */
    __m128 v1 = _mm_shuffle_ps(*(__m128*)&a, *(__m128*)&b, mask1);
    __m128 v2 = _mm_shuffle_ps(*(__m128*)&c, *(__m128*)&d, mask2);
    __m128 v3 = _mm_shuffle_ps(v1, v2, mask3);
    __m128 v4 = _mm_shuffle_ps(v2, v1, mask4);
    
    /* Blend operations add more operands */
    __m128 result = _mm_blend_ps(v3, v4, 0x5);
    return *(v4sf*)&result;
#else
    /* Fallback for non-SSE */
    v4sf result = a + b + c + d;
    return result;
#endif
}

/* Pattern B: Fused multiply-add chain */
NOINLINE static float pattern_b_fma_chain(float a, float b, float c, float d,
                                          float e, float f, float g, float h,
                                          float i, float j, float k) {
#if defined(__FMA__) || defined(__AVX2__)
    /* Deep FMA chain that may flatten to many operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(t1, t2, t3);
    float result = __builtin_fmaf(t4, j, k);
    
    /* Additional arithmetic to prevent simplification */
    result = __builtin_fmaf(result, 1.1f, 2.2f);
    result = __builtin_fmaf(result, 3.3f, 4.4f);
    return result;
#else
    /* Manual FMA emulation */
    return a * b + c + d * e + f + g * h + i + j * k;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Extract each element - creates many extract operations */
    float sum = 0.0f;
    
#if defined(__SSE__)
    /* Use SSE extract operations */
    sum += ((float*)&v1)[0] + ((float*)&v1)[1] + ((float*)&v1)[2] + ((float*)&v1)[3];
    sum += ((float*)&v2)[0] + ((float*)&v2)[1] + ((float*)&v2)[2] + ((float*)&v2)[3];
    sum += ((float*)&v3)[0] + ((float*)&v3)[1] + ((float*)&v3)[2] + ((float*)&v3)[3];
    sum += ((float*)&v4)[0] + ((float*)&v4)[1] + ((float*)&v4)[2] + ((float*)&v4)[3];
    
    /* Additional arithmetic with the extracted values */
    sum = sum * 1.5f - 2.5f;
#else
    /* Fallback */
    for (int i = 0; i < 4; i++) {
        sum += ((float*)&v1)[i] + ((float*)&v2)[i] + ((float*)&v3)[i] + ((float*)&v4)[i];
    }
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations */
NOINLINE static v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                                  v4sf mask1, v4sf mask2) {
#if defined(__SSE__)
    /* Multiple comparisons and blends */
    __m128 cmp1 = _mm_cmpgt_ps(*(__m128*)&a, *(__m128*)&b);
    __m128 cmp2 = _mm_cmplt_ps(*(__m128*)&c, *(__m128*)&d);
    __m128 cmp3 = _mm_cmpeq_ps(*(__m128*)&mask1, *(__m128*)&mask2);
    
    /* Combine masks with logical operations */
    __m128 mask = _mm_and_ps(cmp1, _mm_or_ps(cmp2, cmp3));
    
    /* Conditional select */
    __m128 result = _mm_blendv_ps(*(__m128*)&a, *(__m128*)&b, mask);
    
    /* Additional operation to increase operand count */
    result = _mm_add_ps(result, _mm_mul_ps(*(__m128*)&c, *(__m128*)&d));
    
    return *(v4sf*)&result;
#else
    /* Fallback */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        float cond = (((float*)&a)[i] > ((float*)&b)[i]) &&
                     (((float*)&c)[i] < ((float*)&d)[i]) &&
                     (((float*)&mask1)[i] == ((float*)&mask2)[i]);
        ((float*)&result)[i] = cond ? ((float*)&a)[i] : ((float*)&b)[i];
    }
    return result;
#endif
}

/* Pattern E: Inline assembly with many operands */
NOINLINE static int pattern_e_multi_operand_asm(int a, int b, int c, int d, int e,
                                                int f, int g, int h, int i, int j) {
    int result1, result2;
    
    /* Inline assembly with 10 explicit operands */
    asm volatile (
        "add %[r1], %[a], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "add %[r1], %[r1], %[d]\n\t"
        "add %[r2], %[e], %[f]\n\t"
        "add %[r2], %[r2], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "mul %[r1], %[r1], %[r2]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r1], %[r1], %[j]"
        : [r1] "=r" (result1), [r2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1;
}

/* Pattern F: Complex builtin with immediate and multiple vectors */
NOINLINE static v4sf pattern_f_complex_builtin(v4sf a, v4sf b, v4sf c, v4sf d) {
#if defined(__SSE4_1__)
    /* Use multiple SSE4.1 operations with different immediates */
    __m128 v1 = _mm_dp_ps(*(__m128*)&a, *(__m128*)&b, 0xFF);
    __m128 v2 = _mm_dp_ps(*(__m128*)&c, *(__m128*)&d, 0x7F);
    
    /* Blend with immediate control */
    __m128 result = _mm_blend_ps(v1, v2, 0x3);
    
    /* Round with different modes */
    result = _mm_round_ps(result, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    result = _mm_round_ps(result, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
    
    return *(v4sf*)&result;
#elif defined(__SSE__)
    /* Fallback to SSE */
    __m128 v1 = _mm_add_ps(*(__m128*)&a, *(__m128*)&b);
    __m128 v2 = _mm_add_ps(*(__m128*)&c, *(__m128*)&d);
    __m128 v3 = _mm_mul_ps(v1, v2);
    __m128 v4 = _mm_sub_ps(v1, v2);
    __m128 result = _mm_shuffle_ps(v3, v4, 0x1B);
    return *(v4sf*)&result;
#else
    return a + b + c + d;
#endif
}

/* Main test function that exercises all patterns */
NOINLINE static float run_all_patterns(int argc, char** argv) {
    float total = 0.0f;
    
    /* Initialize test data with some variability based on argc */
    v4sf va = {1.0f * argc, 2.0f, 3.0f, 4.0f};
    v4sf vb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vd = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf mask1 = {1.0f, 0.0f, 1.0f, 0.0f};
    v4sf mask2 = {0.0f, 1.0f, 0.0f, 1.0f};
    
    /* Use different patterns based on argc to ensure all are compiled */
    if (argc > 1) {
        /* Pattern A: Complex shuffle */
        v4sf result_a = pattern_a_shuffle(va, vb, vc, vd, 0x1B, 0x4E, 0x93, 0x27);
        total += ((float*)&result_a)[0] + ((float*)&result_a)[1];
        
        /* Pattern B: FMA chain with 11 arguments */
        float result_b = pattern_b_fma_chain(1.1f, 2.2f, 3.3f, 4.4f, 5.5f,
                                            6.6f, 7.7f, 8.8f, 9.9f, 10.1f, 11.1f);
        total += result_b;
    }
    
    if (argc > 2) {
        /* Pattern C: Vector reduction */
        float result_c = pattern_c_vector_reduce(va, vb, vc, vd);
        total += result_c;
        
        /* Pattern D: Conditional select */
        v4sf result_d = pattern_d_conditional_select(va, vb, vc, vd, mask1, mask2);
        total += ((float*)&result_d)[2] + ((float*)&result_d)[3];
    }
    
    if (argc > 3) {
        /* Pattern E: Multi-operand assembly */
        int result_e = pattern_e_multi_operand_asm(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        total += result_e;
    }
    
    if (argc > 4) {
        /* Pattern F: Complex builtin */
        v4sf result_f = pattern_f_complex_builtin(va, vb, vc, vd);
        total += ((float*)&result_f)[0] + ((float*)&result_f)[1] + 
                 ((float*)&result_f)[2] + ((float*)&result_f)[3];
    }
    
    /* Additional mixed operations to increase operand counts */
    for (int i = 0; i < 4; i++) {
        total = total * 1.1f - 0.5f;
        total += ((float*)&va)[i] * ((float*)&vb)[i];
        total -= ((float*)&vc)[i] / ((float*)&vd)[i];
    }
    
    return total;
}

int main(int argc, char** argv) {
    /* Run the test patterns */
    float result = run_all_patterns(argc, argv);
    
    /* Use the result to prevent dead code elimination */
    sink = (int)result;
    
    /* Print something to ensure execution */
    printf("Test result: %f (sink: %d)\n", result, sink);
    
    return 0;
}
