/* test_optabs.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdlib.h>

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
#else
/* Fallback definitions if SSE not available */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
#endif

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa, used))

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Pattern A: Vector blend with complex mask computation */
NOINLINE static v4sf pattern_a(v4sf a, v4sf b, v4sf c, v4sf d, 
                               int m1, int m2, int m3, int m4) {
#ifdef __SSE__
    /* Complex mask computation that may expand to many operands */
    __m128 mask = _mm_set_ps(m4, m3, m2, m1);
    __m128 cmp1 = _mm_cmplt_ps(a, b);
    __m128 cmp2 = _mm_cmpgt_ps(c, d);
    __m128 mask1 = _mm_and_ps(cmp1, mask);
    __m128 mask2 = _mm_or_ps(cmp2, mask1);
    
    /* Blend operation that may need many operands during expansion */
    __m128 result = _mm_blendv_ps(a, b, mask2);
    return result;
#else
    /* Fallback for non-SSE targets */
    v4sf result = a + b + c + d;
    return result;
#endif
}

/* Pattern B: FMA chain creating deep expression tree */
NOINLINE static float pattern_b(float a, float b, float c, 
                                float d, float e, float f,
                                float g, float h, float i) {
#ifdef __FP_FAST_FMA
    /* Chain of FMA operations that may flatten to many operands */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(t1, t2, t3);
    float t5 = __builtin_fma(t4, a, b);
    float t6 = __builtin_fma(t5, c, d);
    return __builtin_fma(t6, e, f);
#else
    /* Manual FMA emulation */
    return a * b + c * d + e * f + g * h + i;
#endif
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE static float pattern_c(v4sf v) {
#ifdef __SSE__
    /* Extract each element - each extract may add operands */
    float e0 = _mm_cvtss_f32(v);
    float e1 = _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)));
    float e2 = _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2)));
    float e3 = _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3)));
    
    /* Complex reduction that may expand to many operands */
    float sum = e0 + e1 + e2 + e3;
    float prod = e0 * e1 * e2 * e3;
    return sum + prod;
#else
    float *f = (float*)&v;
    return f[0] + f[1] + f[2] + f[3];
#endif
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE static v4sf pattern_d(v4sf a, v4sf b, v4sf c, v4sf d,
                               v4sf e, v4sf f) {
#ifdef __SSE__
    /* Multiple comparisons creating complex condition masks */
    __m128 cmp1 = _mm_cmpeq_ps(a, b);
    __m128 cmp2 = _mm_cmpneq_ps(c, d);
    __m128 cmp3 = _mm_cmplt_ps(e, f);
    __m128 cmp4 = _mm_cmpgt_ps(a, f);
    
    /* Combine masks with logical operations */
    __m128 mask1 = _mm_and_ps(cmp1, cmp2);
    __m128 mask2 = _mm_or_ps(cmp3, cmp4);
    __m128 final_mask = _mm_xor_ps(mask1, mask2);
    
    /* Select based on complex mask */
    __m128 result = _mm_blendv_ps(a, b, final_mask);
    result = _mm_blendv_ps(result, c, _mm_and_ps(mask1, mask2));
    
    return result;
#else
    return a + b + c + d + e + f;
#endif
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOINLINE static int pattern_e(int a, int b, int c, int d, int e,
                              int f, int g, int h, int i, int j) {
    int result;
    
    /* Inline asm with 10 inputs and 1 output = 11 total operands */
    asm volatile (
        "add %[a], %[b]\n\t"
        "add %[c], %[d]\n\t"
        "add %[e], %[f]\n\t"
        "add %[g], %[h]\n\t"
        "add %[i], %[j]\n\t"
        "imul %[b], %[d]\n\t"
        "imul %[f], %[h]\n\t"
        "add %[d], %[h]\n\t"
        "add %[h], %[j]\n\t"
        "mov %[j], %[result]"
        : [result] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result;
}

/* Pattern F: Vector shuffle with many immediate operands */
NOINLINE static v4sf pattern_f(v4sf a, v4sf b, v4sf c, v4sf d) {
#ifdef __SSE__
    /* Complex shuffle pattern that may decompose to many RTL operands */
    __m128 t1 = _mm_shuffle_ps(a, b, _MM_SHUFFLE(3, 2, 1, 0));
    __m128 t2 = _mm_shuffle_ps(c, d, _MM_SHUFFLE(0, 1, 2, 3));
    __m128 t3 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 t4 = _mm_shuffle_ps(a, c, _MM_SHUFFLE(1, 0, 3, 2));
    __m128 t5 = _mm_shuffle_ps(b, d, _MM_SHUFFLE(2, 3, 0, 1));
    
    return _mm_shuffle_ps(t3, _mm_add_ps(t4, t5), _MM_SHUFFLE(3, 1, 2, 0));
#else
    return a + b + c + d;
#endif
}

/* Main test driver with runtime variability */
int main(int argc, char *argv[]) {
    float result = 0.0f;
    
    /* Initialize with some values */
    v4sf va = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vc = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vd = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf ve = {17.0f, 18.0f, 19.0f, 20.0f};
    v4sf vf = {21.0f, 22.0f, 23.0f, 24.0f};
    
    /* Use argc to select different patterns, ensuring all get compiled */
    switch (argc % 6) {
        case 0:
            result = pattern_c(pattern_a(va, vb, vc, vd, 1, 2, 3, 4))[0];
            break;
        case 1:
            result = pattern_b(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f);
            break;
        case 2:
            result = pattern_c(va)[0];
            break;
        case 3:
            result = pattern_d(va, vb, vc, vd, ve, vf)[0];
            break;
        case 4:
            result = (float)pattern_e(argc, argc+1, argc+2, argc+3, argc+4,
                                      argc+5, argc+6, argc+7, argc+8, argc+9);
            break;
        case 5:
            result = pattern_f(va, vb, vc, vd)[0];
            break;
    }
    
    /* Use result to prevent optimization */
    sink = (int)result;
    
    return sink > 0 ? 0 : 1;
}
