/* test_optabs_high_operand_count.c
 * 
 * This program is designed to trigger GCC's RTL expansion for operations
 * requiring exactly 10 or 11 operands, specifically targeting uncovered
 * switch cases in optabs.cc lines 8254-8263.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define vector types for various architectures */
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

/* Prevent optimization and ensure expansion */
#define NOINLINE __attribute__((noinline, noipa))
#define USED __attribute__((used))

/* Volatile sink to prevent optimization */
static volatile int sink;

/* Pattern A: Complex vector shuffle with many operands */
NOINLINE USED
v4sf pattern_a_shuffle_many_operands(float a0, float a1, float a2, float a3,
                                      float b0, float b1, float b2, float b3,
                                      int mask0, int mask1, int mask2, int mask3) {
    /* Construct vectors from many scalar inputs */
    v4sf va = {a0, a1, a2, a3};
    v4sf vb = {b0, b1, b2, b3};
    
    /* Complex shuffle operation that may expand to many operands */
#ifdef __SSE__
    /* __builtin_ia32_shufps typically expands with multiple operands */
    v4sf result = __builtin_ia32_shufps(va, vb, 
                                        (mask0 & 3) | ((mask1 & 3) << 2) |
                                        ((mask2 & 3) << 4) | ((mask3 & 3) << 6));
    
    /* Additional operations to increase operand count */
    result = __builtin_ia32_addps(result, va);
    result = __builtin_ia32_mulps(result, vb);
    result = __builtin_ia32_shufps(result, result, 0x1B); /* Another shuffle */
#else
    /* Portable fallback */
    v4sf result = va + vb;
    result = result * va;
#endif
    
    return result;
}

/* Pattern B: Fused multiply-add chain creating deep expression tree */
NOINLINE USED
float pattern_b_fma_chain(float a, float b, float c, float d, float e,
                          float f, float g, float h, float i, float j) {
    /* Chain of operations that may flatten to many operands */
#ifdef __FP_FAST_FMA
    /* FMA operations create complex RTL patterns */
    float t1 = __builtin_fma(a, b, c);
    float t2 = __builtin_fma(d, e, f);
    float t3 = __builtin_fma(g, h, i);
    float t4 = __builtin_fma(t1, t2, t3);
    float result = __builtin_fma(t4, j, a + b + c + d);
#else
    /* Manual FMA simulation */
    float t1 = a * b + c;
    float t2 = d * e + f;
    float t3 = g * h + i;
    float t4 = t1 * t2 + t3;
    float result = t4 * j + (a + b + c + d);
#endif
    
    /* Additional arithmetic to increase complexity */
    result = result * 2.0f - 1.0f;
    result = result / (result + 1.0f);
    
    return result;
}

/* Pattern C: Vector reduction with explicit scalarization */
NOINLINE USED
float pattern_c_vector_reduction(v4sf v1, v4sf v2, v4sf v3, v4sf v4) {
    /* Manual horizontal addition with many extract operations */
    float sum = 0.0f;
    
    /* Extract each element - each extract adds operands */
#ifdef __SSE__
    sum += ((float*)&v1)[0] + ((float*)&v1)[1] + ((float*)&v1)[2] + ((float*)&v1)[3];
    sum += ((float*)&v2)[0] + ((float*)&v2)[1] + ((float*)&v2)[2] + ((float*)&v2)[3];
    sum += ((float*)&v3)[0] + ((float*)&v3)[1] + ((float*)&v3)[2] + ((float*)&v3)[3];
    sum += ((float*)&v4)[0] + ((float*)&v4)[1] + ((float*)&v4)[2] + ((float*)&v4)[3];
#else
    /* Portable extraction */
    float* f1 = (float*)&v1;
    float* f2 = (float*)&v2;
    float* f3 = (float*)&v3;
    float* f4 = (float*)&v4;
    
    for (int i = 0; i < 4; i++) {
        sum += f1[i] + f2[i] + f3[i] + f4[i];
    }
#endif
    
    /* Chain of operations on the sum */
    sum = sum * sum - sum;
    sum = sum / (sum + 1.0f);
    sum = sum * 2.0f - sum / 2.0f;
    
    return sum;
}

/* Pattern D: Conditional vector operations with many comparisons */
NOINLINE USED
v4sf pattern_d_conditional_blend(v4sf a, v4sf b, v4sf c, v4sf d,
                                 v4sf thresh1, v4sf thresh2) {
    v4sf result;
    
#ifdef __SSE__
    /* Multiple comparisons create many operands */
    v4sf cmp1 = __builtin_ia32_cmpltps(a, thresh1);
    v4sf cmp2 = __builtin_ia32_cmpgtps(b, thresh2);
    v4sf cmp3 = __builtin_ia32_cmpeqps(c, d);
    
    /* Complex blend logic */
    v4sf tmp1 = __builtin_ia32_andps(a, cmp1);
    v4sf tmp2 = __builtin_ia32_andps(b, cmp2);
    v4sf tmp3 = __builtin_ia32_andnotps(cmp3, c);
    
    /* More operations */
    tmp1 = __builtin_ia32_addps(tmp1, tmp2);
    tmp3 = __builtin_ia32_mulps(tmp3, d);
    
    /* Final blend */
    v4sf blend_mask = __builtin_ia32_orps(cmp1, cmp2);
    blend_mask = __builtin_ia32_xorps(blend_mask, cmp3);
    
    result = __builtin_ia32_blendvps(tmp1, tmp3, blend_mask);
#else
    /* Portable version */
    result = a + b - c * d;
#endif
    
    return result;
}

/* Pattern E: Inline assembly with exactly 10-11 operands */
NOINLINE USED
int64_t pattern_e_multi_operand_asm(int64_t a, int64_t b, int64_t c, int64_t d,
                                    int64_t e, int64_t f, int64_t g, int64_t h,
                                    int64_t i, int64_t j) {
    int64_t result1, result2;
    
    /* Inline assembly with 10 explicit operands */
    asm volatile (
        /* Operation with 10 operands */
        "imulq %[a], %[b]\n\t"
        "addq %[c], %[b]\n\t"
        "subq %[d], %[b]\n\t"
        "imulq %[e], %[b]\n\t"
        "addq %[f], %[b]\n\t"
        "subq %[g], %[b]\n\t"
        "imulq %[h], %[b]\n\t"
        "addq %[i], %[b]\n\t"
        "movq %[b], %[res1]\n\t"
        : [res1] "=r" (result1), [b] "+r" (b)
        : [a] "r" (a), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g),
          [h] "r" (h), [i] "r" (i)
        : "cc"
    );
    
    /* Another asm with 11 operands */
    asm volatile (
        /* Complex operation chain */
        "leaq (%[a], %[b], 2), %[res2]\n\t"
        "addq %[c], %[res2]\n\t"
        "subq %[d], %[res2]\n\t"
        "imulq %[e], %[res2]\n\t"
        "addq %[f], %[res2]\n\t"
        "subq %[g], %[res2]\n\t"
        "imulq %[h], %[res2]\n\t"
        "addq %[i], %[res2]\n\t"
        "subq %[j], %[res2]\n\t"
        : [res2] "=r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2;
}

/* Pattern F: Complex integer arithmetic with many intermediate values */
NOINLINE USED
int pattern_f_multi_operand_int(int a, int b, int c, int d, int e,
                                int f, int g, int h, int i, int j,
                                int k, int l, int m, int n) {
    /* Many operations that may be combined during expansion */
    int t1 = a * b + c;
    int t2 = d * e - f;
    int t3 = g * h / (i + 1);
    int t4 = j * k % (l + 1);
    int t5 = m * n ^ t1;
    
    /* Chain operations */
    int result = t1 + t2;
    result = result * t3;
    result = result - t4;
    result = result ^ t5;
    result = result & (a | b | c | d);
    result = result | (e & f & g & h);
    
    /* More operations to increase operand count */
    result = (result << 2) | (result >> 30);
    result = result + (i * j * k * l);
    result = result - (m * n * a * b);
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    float checksum = 0.0f;
    
    /* Initialize with some values, using argc for variability */
    float base = (argc > 1) ? (float)atoi(argv[1]) : 1.5f;
    
    /* Test Pattern A */
    {
        v4sf result = pattern_a_shuffle_many_operands(
            base + 1.0f, base + 2.0f, base + 3.0f, base + 4.0f,
            base + 5.0f, base + 6.0f, base + 7.0f, base + 8.0f,
            argc, argc + 1, argc + 2, argc + 3
        );
        
        float* r = (float*)&result;
        checksum += r[0] + r[1] + r[2] + r[3];
    }
    
    /* Test Pattern B */
    {
        float result = pattern_b_fma_chain(
            base, base * 2.0f, base * 3.0f, base * 4.0f, base * 5.0f,
            base * 6.0f, base * 7.0f, base * 8.0f, base * 9.0f, base * 10.0f
        );
        checksum += result;
    }
    
    /* Test Pattern C */
    {
        v4sf v1 = {base, base + 1.0f, base + 2.0f, base + 3.0f};
        v4sf v2 = {base + 4.0f, base + 5.0f, base + 6.0f, base + 7.0f};
        v4sf v3 = {base + 8.0f, base + 9.0f, base + 10.0f, base + 11.0f};
        v4sf v4 = {base + 12.0f, base + 13.0f, base + 14.0f, base + 15.0f};
        
        float result = pattern_c_vector_reduction(v1, v2, v3, v4);
        checksum += result;
    }
    
    /* Test Pattern D */
    {
        v4sf a = {base, base + 1.0f, base + 2.0f, base + 3.0f};
        v4sf b = {base + 4.0f, base + 5.0f, base + 6.0f, base + 7.0f};
        v4sf c = {base + 8.0f, base + 9.0f, base + 10.0f, base + 11.0f};
        v4sf d = {base + 12.0f, base + 13.0f, base + 14.0f, base + 15.0f};
        v4sf thresh1 = {5.0f, 5.0f, 5.0f, 5.0f};
        v4sf thresh2 = {10.0f, 10.0f, 10.0f, 10.0f};
        
        v4sf result = pattern_d_conditional_blend(a, b, c, d, thresh1, thresh2);
        float* r = (float*)&result;
        checksum += r[0] + r[1] + r[2] + r[3];
    }
    
    /* Test Pattern E */
    {
        int64_t result = pattern_e_multi_operand_asm(
            (int64_t)(base * 100), (int64_t)(base * 200),
            (int64_t)(base * 300), (int64_t)(base * 400),
            (int64_t)(base * 500), (int64_t)(base * 600),
            (int64_t)(base * 700), (int64_t)(base * 800),
            (int64_t)(base * 900), (int64_t)(base * 1000)
        );
        checksum += (float)(result % 1000);
    }
    
    /* Test Pattern F */
    {
        int result = pattern_f_multi_operand_int(
            (int)base, (int)(base * 2), (int)(base * 3), (int)(base * 4),
            (int)(base * 5), (int)(base * 6), (int)(base * 7), (int)(base * 8),
            (int)(base * 9), (int)(base * 10), (int)(base * 11), (int)(base * 12),
            (int)(base * 13), (int)(base * 14)
        );
        checksum += (float)result;
    }
    
    /* Use checksum to prevent optimization */
    sink = (int)checksum;
    
    printf("Checksum: %f\n", checksum);
    
    return (checksum > 0.0f) ? 0 : 1;
}
