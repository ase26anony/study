/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdlib.h>

/* Prevent aggressive optimization */
static volatile int sink;

/* Generic vector types for portability */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef int v8si __attribute__((vector_size(32)));

/* Force functions to be expanded, not optimized away */
#define NOOPT __attribute__((noinline, noipa, used))

/* SSE/AVX intrinsics if available */
#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef __SSE3__
#include <pmmintrin.h>
#endif

#ifdef __SSSE3__
#include <tmmintrin.h>
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

#ifdef __AVX__
#include <immintrin.h>
#endif

/* ARM NEON intrinsics if available */
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

/* Pattern A: Complex vector shuffle with many operands */
NOOPT v4sf pattern_a_shuffle_many_operands(v4sf a, v4sf b, v4sf c, v4sf d,
                                          int m1, int m2, int m3, int m4) {
#ifdef __SSE__
    /* Create a complex shuffle pattern that may expand to many operands */
    __m128 v1 = _mm_shuffle_ps(a, b, m1);
    __m128 v2 = _mm_shuffle_ps(c, d, m2);
    __m128 v3 = _mm_shuffle_ps(v1, v2, m3);
    __m128 v4 = _mm_shuffle_ps(v3, _mm_setzero_ps(), m4);
    
    /* Mix with arithmetic to prevent elimination */
    v4 = _mm_add_ps(v4, _mm_mul_ps(a, b));
    v4 = _mm_sub_ps(v4, _mm_mul_ps(c, d));
    
    return v4;
#else
    /* Fallback for non-SSE targets */
    v4sf result = a + b - c * d;
    sink = m1 + m2 + m3 + m4;
    return result;
#endif
}

/* Pattern B: FMA chain creating deep expression tree */
NOOPT float pattern_b_fma_chain(float a, float b, float c, float d,
                               float e, float f, float g, float h,
                               float i, float j, float k, float l) {
#ifdef __FMA__
    /* Chain of FMA operations that may flatten to many operands */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    
    float t5 = __builtin_fmaf(t1, t2, t3);
    float t6 = __builtin_fmaf(t4, t5, a + b);
    
    return __builtin_fmaf(t6, t1 - t2, t3 * t4);
#else
    /* Manual FMA emulation */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = j * k + l;
    
    float t5 = t1 * t2 + t3;
    float t6 = t4 * t5 + (a + b);
    
    return t6 * (t1 - t2) + (t3 * t4);
#endif
}

/* Pattern C: Vector extraction and horizontal sum with many extracts */
NOOPT float pattern_c_many_extracts(v4sf v) {
    float sum = 0.0f;
    
#ifdef __SSE__
    /* Extract each element individually - each extract is an operation */
    sum += ((float*)&v)[0];  /* Alternative to _mm_extract_ps for portability */
    sum += ((float*)&v)[1];
    sum += ((float*)&v)[2];
    sum += ((float*)&v)[3];
    
    /* Additional arithmetic to create more operands */
    sum = sum * 2.0f - ((float*)&v)[0];
    sum = sum / 1.5f + ((float*)&v)[1];
    sum = sum * 3.0f - ((float*)&v)[2];
    sum = sum / 2.5f + ((float*)&v)[3];
#else
    /* Portable version */
    for (int i = 0; i < 4; i++) {
        sum += ((float*)&v)[i];
    }
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOOPT v4sf pattern_d_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                      v4sf mask1, v4sf mask2) {
    v4sf result;
    
#ifdef __SSE4_1__
    /* Multiple comparison and blend operations */
    __m128 cmp1 = _mm_cmpgt_ps(a, b);
    __m128 cmp2 = _mm_cmplt_ps(c, d);
    __m128 cmp3 = _mm_cmpeq_ps(mask1, mask2);
    
    __m128 blend1 = _mm_blendv_ps(a, b, cmp1);
    __m128 blend2 = _mm_blendv_ps(c, d, cmp2);
    __m128 blend3 = _mm_blendv_ps(blend1, blend2, cmp3);
    
    /* Additional operations to increase operand count */
    blend3 = _mm_add_ps(blend3, _mm_mul_ps(cmp1, cmp2));
    blend3 = _mm_sub_ps(blend3, _mm_div_ps(a, b));
    
    result = blend3;
#else
    /* Portable fallback */
    result = a > b ? a : b;
    result = result + (c < d ? c : d);
    result = result * (mask1 == mask2 ? mask1 : mask2);
#endif
    
    return result;
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOOPT int64_t pattern_e_many_operand_asm(int64_t a, int64_t b, int64_t c,
                                        int64_t d, int64_t e, int64_t f,
                                        int64_t g, int64_t h, int64_t i,
                                        int64_t j, int64_t k) {
    int64_t result1, result2, result3;
    
    /* Inline assembly with many input/output operands */
    asm volatile (
        /* Complex operation with 11 total operands (10 inputs + 1 output) */
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[b]\n\t"
        "sub %[r1], %[c]\n\t"
        "imul %[r1], %[d]\n\t"
        "add %[r1], %[e]\n\t"
        "sub %[r1], %[f]\n\t"
        "add %[r1], %[g]\n\t"
        "sub %[r1], %[h]\n\t"
        "add %[r1], %[i]\n\t"
        "sub %[r1], %[j]\n\t"
        "add %[r1], %[k]"
        : [r1] "=&r" (result1),
          [r2] "=&r" (result2),
          [r3] "=&r" (result3)
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [e] "r" (e),
          [f] "r" (f),
          [g] "r" (g),
          [h] "r" (h),
          [i] "r" (i),
          [j] "r" (j),
          [k] "r" (k)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Pattern F: AVX2 gather operation (if available) - often expands to many operands */
NOOPT v8sf pattern_f_gather_like(v8sf base, v8si indices, v8sf scale) {
    v8sf result = {0};
    
#ifdef __AVX2__
    /* Simulate gather-like pattern with many memory operands */
    for (int i = 0; i < 8; i++) {
        int idx = ((int*)&indices)[i] & 7;
        ((float*)&result)[i] = ((float*)&base)[idx] * ((float*)&scale)[i];
    }
    
    /* Additional operations to increase complexity */
    result = result + base * 2.0f;
    result = result - scale / 3.0f;
#else
    /* Portable version */
    for (int i = 0; i < 8; i++) {
        int idx = ((int*)&indices)[i] & 7;
        ((float*)&result)[i] = ((float*)&base)[idx];
    }
#endif
    
    return result;
}

/* Main test driver that exercises all patterns */
int main(int argc, char *argv[]) {
    float checksum = 0.0f;
    
    /* Initialize with some data */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    v4sf mask1 = {1.0f, 0.0f, 1.0f, 0.0f};
    v4sf mask2 = {0.0f, 1.0f, 0.0f, 1.0f};
    
    v8sf vec8_1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8si indices = {0, 1, 2, 3, 4, 5, 6, 7};
    v8sf scale8 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Use argc to add variability and prevent dead code elimination */
    int selector = argc > 1 ? atoi(argv[1]) % 6 : 0;
    
    switch (selector) {
        case 0:
            /* Pattern A: Shuffle with many operands */
            checksum += ((float*)&pattern_a_shuffle_many_operands(
                vec1, vec2, vec3, vec4, 0x1B, 0x27, 0x39, 0x4E))[0];
            break;
            
        case 1:
            /* Pattern B: FMA chain - 12 float arguments */
            checksum += pattern_b_fma_chain(
                1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f,
                7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f);
            break;
            
        case 2:
            /* Pattern C: Many extract operations */
            checksum += pattern_c_many_extracts(vec1);
            break;
            
        case 3:
            /* Pattern D: Conditional blend with comparisons */
            checksum += ((float*)&pattern_d_conditional_blend(
                vec1, vec2, vec3, vec4, mask1, mask2))[0];
            break;
            
        case 4:
            /* Pattern E: Inline assembly with 11 operands */
            checksum += (float)pattern_e_many_operand_asm(
                1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
            break;
            
        case 5:
            /* Pattern F: Gather-like pattern */
            checksum += ((float*)&pattern_f_gather_like(vec8_1, indices, scale8))[0];
            break;
    }
    
    /* Use checksum to prevent optimization */
    sink = (int)checksum;
    
    return sink != 0 ? 0 : 1;
}
