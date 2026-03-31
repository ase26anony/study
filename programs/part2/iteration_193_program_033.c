/* test_optabs_coverage.c - Test program to cover 10/11 operand switch cases in optabs.cc */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical functions */
#define NOOPT __attribute__((noinline, noipa, used))

/* Define vector types for portability */
#ifdef __SSE__
#include <xmmintrin.h>
typedef __m128 v4sf;
typedef __m128i v4si;
#else
/* Fallback definitions */
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

/* Volatile variable to prevent dead code elimination */
static volatile int g_volatile_result = 0;

/* Pattern A: Complex vector shuffle with many operands */
NOOPT v4sf pattern_a_shuffle(v4sf a, v4sf b, v4sf c, v4sf d, 
                            int imm1, int imm2, int imm3, int imm4) {
    /* Create a complex shuffle pattern that may expand to many RTL operands */
    v4sf temp1, temp2, temp3, result;
    
    /* Multiple shuffle operations chained together */
#ifdef __SSE__
    temp1 = _mm_shuffle_ps(a, b, imm1);
    temp2 = _mm_shuffle_ps(c, d, imm2);
    temp3 = _mm_shuffle_ps(temp1, temp2, imm3);
    result = _mm_shuffle_ps(temp3, _mm_setzero_ps(), imm4);
#else
    /* Portable fallback */
    temp1 = __builtin_shufflevector(a, b, 0, 1, 4, 5);
    temp2 = __builtin_shufflevector(c, d, 2, 3, 6, 7);
    temp3 = __builtin_shufflevector(temp1, temp2, 0, 2, 4, 6);
    result = __builtin_shufflevector(temp3, (v4sf){0}, 3, 2, 1, 0);
#endif
    
    return result;
}

/* Pattern B: Fused multiply-add chain with many accumulators */
NOOPT float pattern_b_fma_chain(float a, float b, float c, float d,
                               float e, float f, float g, float h,
                               float i, float j, float k, float l) {
    /* Deep FMA expression tree that may flatten to many operands */
#ifdef __FMA__
    /* Use builtin_fma if available */
    float t1 = __builtin_fmaf(a, b, c);
    float t2 = __builtin_fmaf(d, e, f);
    float t3 = __builtin_fmaf(g, h, i);
    float t4 = __builtin_fmaf(j, k, l);
    
    float t5 = __builtin_fmaf(t1, t2, t3);
    float result = __builtin_fmaf(t4, t5, t1 + t2 + t3 + t4);
#else
    /* Manual FMA simulation */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = j * k + l;
    
    float t5 = t1 * t2 + t3;
    float result = t4 * t5 + (t1 + t2 + t3 + t4);
#endif
    
    return result;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOOPT float pattern_c_vector_reduce(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manually unrolled horizontal addition with many extracts */
    float sum = 0.0f;
    
    /* Extract and sum 16 elements (4 vectors × 4 lanes) */
#ifdef __SSE__
    sum += _mm_cvtss_f32(v1);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 3, 3, 3)));
    
    sum += _mm_cvtss_f32(v2);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 3, 3, 3)));
    
    sum += _mm_cvtss_f32(v3);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v3, v3, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v3, v3, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v3, v3, _MM_SHUFFLE(3, 3, 3, 3)));
    
    sum += _mm_cvtss_f32(v4);
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v4, v4, _MM_SHUFFLE(1, 1, 1, 1)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v4, v4, _MM_SHUFFLE(2, 2, 2, 2)));
    sum += _mm_cvtss_f32(_mm_shuffle_ps(v4, v4, _MM_SHUFFLE(3, 3, 3, 3)));
#else
    /* Portable extraction */
    float* f1 = (float*)&v1;
    float* f2 = (float*)&v2;
    float* f3 = (float*)&v3;
    float* f4 = (float*)&v4;
    
    for (int i = 0; i < 4; i++) sum += f1[i];
    for (int i = 0; i < 4; i++) sum += f2[i];
    for (int i = 0; i < 4; i++) sum += f3[i];
    for (int i = 0; i < 4; i++) sum += f4[i];
#endif
    
    return sum;
}

/* Pattern D: Conditional vector operations with complex mask */
NOOPT v4sf pattern_d_conditional_select(v4sf a, v4sf b, v4sf c, v4sf d,
                                       v4sf mask1, v4sf mask2) {
    v4sf result;
    
#ifdef __SSE__
    /* Complex conditional logic that may expand to many operands */
    v4sf cmp1 = _mm_cmplt_ps(a, b);
    v4sf cmp2 = _mm_cmpgt_ps(c, d);
    v4sf cmp3 = _mm_cmpeq_ps(mask1, mask2);
    
    /* Combine masks with logical operations */
    v4sf mask = _mm_or_ps(_mm_and_ps(cmp1, cmp2), cmp3);
    
    /* Conditional select */
    result = _mm_or_ps(_mm_and_ps(mask, a), _mm_andnot_ps(mask, b));
    
    /* Additional operations to increase operand count */
    result = _mm_add_ps(result, _mm_mul_ps(c, d));
    result = _mm_sub_ps(result, _mm_div_ps(a, _mm_add_ps(b, (v4sf){1.0f})));
#else
    /* Portable version */
    result = a;
#endif
    
    return result;
}

/* Pattern E: Inline assembly with exactly 11 operands */
NOOPT int64_t pattern_e_multi_operand_asm(int64_t a, int64_t b, int64_t c,
                                         int64_t d, int64_t e, int64_t f,
                                         int64_t g, int64_t h, int64_t i,
                                         int64_t j, int64_t k) {
    int64_t result1, result2, result3;
    
    /* Inline assembly with many input/output operands */
    asm volatile (
        /* 11 explicit operands: 8 inputs, 3 outputs */
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r3], %[e], %[f]\n\t"
        "add %[r1], %[r1], %[g]\n\t"
        "add %[r2], %[r2], %[h]\n\t"
        "add %[r3], %[r3], %[i]\n\t"
        "mul %[r1], %[r1], %[j]\n\t"
        "mul %[r2], %[r2], %[k]\n\t"
        "add %[out], %[r1], %[r2]\n\t"
        "add %[out], %[out], %[r3]"
        : [out] "=r" (result1), [r1] "=&r" (result2), [r2] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    return result1;
}

/* Main test driver */
int main(int argc, char** argv) {
    int checksum = 0;
    
    /* Initialize test data with some variability based on argc */
    float fa = 1.0f + argc;
    float fb = 2.0f + argc;
    float fc = 3.0f + argc;
    float fd = 4.0f + argc;
    float fe = 5.0f + argc;
    float ff = 6.0f + argc;
    float fg = 7.0f + argc;
    float fh = 8.0f + argc;
    
    v4sf va = {fa, fb, fc, fd};
    v4sf vb = {fe, ff, fg, fh};
    v4sf vc = {fb, fc, fd, fe};
    v4sf vd = {ff, fg, fh, fa};
    
    /* Test Pattern A: Complex shuffle */
    v4sf result_a = pattern_a_shuffle(va, vb, vc, vd, 0x1B, 0x27, 0x39, 0x4E);
    checksum += (int)(((float*)&result_a)[0] * 100);
    
    /* Test Pattern B: FMA chain */
    float result_b = pattern_b_fma_chain(fa, fb, fc, fd, fe, ff, fg, fh,
                                        fa*2, fb*2, fc*2, fd*2);
    checksum += (int)(result_b * 100);
    
    /* Test Pattern C: Vector reduction */
    float result_c = pattern_c_vector_reduce(va, vb, vc, vd);
    checksum += (int)(result_c * 100);
    
    /* Test Pattern D: Conditional select */
    v4sf result_d = pattern_d_conditional_select(va, vb, vc, vd, va, vb);
    checksum += (int)(((float*)&result_d)[0] * 100);
    
    /* Test Pattern E: Multi-operand assembly */
    int64_t ia = argc + 1;
    int64_t ib = argc + 2;
    int64_t ic = argc + 3;
    int64_t id = argc + 4;
    int64_t ie = argc + 5;
    int64_t _if = argc + 6;
    int64_t ig = argc + 7;
    int64_t ih = argc + 8;
    int64_t ii = argc + 9;
    int64_t ij = argc + 10;
    int64_t ik = argc + 11;
    
    int64_t result_e = pattern_e_multi_operand_asm(ia, ib, ic, id, ie, _if,
                                                  ig, ih, ii, ij, ik);
    checksum += (int)(result_e % 1000);
    
    /* Store result to volatile to prevent optimization */
    g_volatile_result = checksum;
    
    /* Print result to ensure code isn't dead */
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
