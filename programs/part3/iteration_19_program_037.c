/* test_optabs_10_11_operands.c
 * 
 * This program aims to trigger GCC's RTL expansion for instructions
 * with 10 or 11 operands, covering the uncovered blocks in optabs.cc.
 * 
 * Compile with: gcc -O2 -fdump-rtl-expand -c test_optabs_10_11_operands.c
 * Check the .expand file for multi-operand RTL patterns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations from removing our test code */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* ==================== TARGET-SPECIFIC CODE ==================== */

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

/* Complex inline assembly that might expand to many operands */
NOINLINE static uint64_t x86_multi_operand_asm(uint64_t a, uint64_t b, 
                                               uint64_t c, uint64_t d,
                                               uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h,
                                               uint64_t i, uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Extended inline assembly with many operands */
    asm volatile (
        /* Complex operation with 10 explicit operands */
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "imul %[c], %[r1]\n\t"
        "add %[d], %[r1]\n\t"
        "sub %[e], %[r1]\n\t"
        "xor %[f], %[r1]\n\t"
        "or %[g], %[r1]\n\t"
        "and %[h], %[r1]\n\t"
        "add %[i], %[r1]\n\t"
        "sub %[j], %[r1]\n\t"
        "mov %[r1], %[r2]\n\t"
        "shl $3, %[r2]\n\t"
        "add %[r2], %[r1]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc", "memory"
    );
    
    return result1 + result2 + result3;
}

/* AVX-512 operation that might expand to many operands */
NOINLINE static __m512i avx512_complex_op(__m512i a, __m512i b, __m512i c,
                                          __m512i d, __m512i e, __m512i f) {
    /* Complex chain of operations */
    __m512i t1 = _mm512_add_epi64(a, b);
    __m512i t2 = _mm512_sub_epi64(c, d);
    __m512i t3 = _mm512_mullo_epi64(t1, t2);
    __m512i t4 = _mm512_add_epi64(e, f);
    __m512i t5 = _mm512_slli_epi64(t3, 2);
    __m512i t6 = _mm512_add_epi64(t4, t5);
    __m512i t7 = _mm512_srli_epi64(t6, 1);
    
    /* Masked operation adds more operands */
    __mmask8 mask = 0xFF;
    __m512i result = _mm512_mask_add_epi64(t7, mask, t6, t3);
    
    return result;
}

#elif defined(__aarch64__)
#include <arm_neon.h>

/* ARM NEON complex operation */
NOINLINE static int32x4_t aarch64_multi_operand(int32x4_t a, int32x4_t b,
                                                int32x4_t c, int32x4_t d,
                                                int32x4_t e, int32x4_t f,
                                                int32x4_t g, int32x4_t h) {
    /* Complex sequence that might expand to many operands */
    int32x4_t t1 = vaddq_s32(a, b);
    int32x4_t t2 = vsubq_s32(c, d);
    int32x4_t t3 = vmulq_s32(t1, t2);
    int32x4_t t4 = vaddq_s32(e, f);
    int32x4_t t5 = vaddq_s32(g, h);
    int32x4_t t6 = vmlaq_s32(t3, t4, t5);  /* t3 + t4 * t5 */
    
    /* Lane operations add complexity */
    int32x2_t lane1 = vget_low_s32(t6);
    int32x2_t lane2 = vget_high_s32(t6);
    int32x2_t t7 = vadd_s32(lane1, lane2);
    int32x4_t result = vcombine_s32(t7, t7);
    
    return result;
}

#elif defined(__powerpc64__) || defined(__PPC64__)
/* PowerPC specific operations */
NOINLINE static vector unsigned long long ppc_multi_operand(
    vector unsigned long long a, vector unsigned long long b,
    vector unsigned long long c, vector unsigned long long d,
    vector unsigned long long e, vector unsigned long long f) {
    
    /* Complex vector operations */
    vector unsigned long long t1 = vec_add(a, b);
    vector unsigned long long t2 = vec_sub(c, d);
    vector unsigned long long t3 = vec_mul(t1, t2);
    vector unsigned long long t4 = vec_add(e, f);
    vector unsigned long long t5 = vec_add(t3, t4);
    
    /* Permute operation adds operand complexity */
    vector unsigned char perm = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    vector unsigned long long result = vec_perm(t5, t3, perm);
    
    return result;
}

#else
/* Generic fallback - still try to create complex expressions */
NOINLINE static uint64_t generic_multi_operand(uint64_t a, uint64_t b,
                                               uint64_t c, uint64_t d,
                                               uint64_t e, uint64_t f,
                                               uint64_t g, uint64_t h,
                                               uint64_t i, uint64_t j) {
    /* Complex expression that might expand to many RTL operands */
    uint64_t t1 = (a + b) * (c - d);
    uint64_t t2 = (e | f) & (g ^ h);
    uint64_t t3 = (i << 4) | (j >> 4);
    uint64_t t4 = (t1 + t2) * t3;
    uint64_t t5 = (t4 << 2) + (t4 >> 2);
    
    /* Multi-precision arithmetic */
    uint64_t hi1, lo1, hi2, lo2;
    lo1 = (t1 & 0xFFFFFFFF) * (t2 & 0xFFFFFFFF);
    hi1 = ((t1 >> 32) * (t2 & 0xFFFFFFFF)) + ((t1 & 0xFFFFFFFF) * (t2 >> 32));
    
    lo2 = (t3 & 0xFFFFFFFF) * (t4 & 0xFFFFFFFF);
    hi2 = ((t3 >> 32) * (t4 & 0xFFFFFFFF)) + ((t3 & 0xFFFFFFFF) * (t4 >> 32));
    
    uint64_t result = (hi1 + hi2) + ((lo1 + lo2) >> 32);
    return result;
}
#endif

/* ==================== COMMON TEST FUNCTIONS ==================== */

/* Function that uses vector extensions to create complex operations */
NOINLINE static v4si vector_complex_op(v4si a, v4si b, v4si c, v4si d,
                                       v4si e, v4si f, v4si g, v4si h) {
    /* Complex vector expression that might expand to many operands */
    v4si t1 = a + b;
    v4si t2 = c - d;
    v4si t3 = t1 * t2;
    v4si t4 = e & f;
    v4si t5 = g | h;
    v4si t6 = t4 ^ t5;
    v4si t7 = t3 << 2;
    v4si t8 = t6 >> 1;
    v4si t9 = t7 + t8;
    v4si t10 = t9 * a;
    
    /* Permutation-like operation using GCC extensions */
    v4si result = __builtin_shuffle(t10, t3, (v4si){3, 2, 1, 0});
    
    return result;
}

/* Multi-precision multiplication that might expand to many operands */
NOINLINE static uint64_t expand_mult_highpart_test(uint64_t a, uint64_t b) {
    /* This mimics what expand_mult_highpart might do */
    uint64_t a_hi = a >> 32;
    uint64_t a_lo = a & 0xFFFFFFFF;
    uint64_t b_hi = b >> 32;
    uint64_t b_lo = b & 0xFFFFFFFF;
    
    /* Complex multiplication that might use many temporaries */
    uint64_t t1 = a_lo * b_lo;
    uint64_t t2 = a_hi * b_lo;
    uint64_t t3 = a_lo * b_hi;
    uint64_t t4 = a_hi * b_hi;
    
    uint64_t lo = t1 & 0xFFFFFFFF;
    uint64_t mid1 = (t1 >> 32) + (t2 & 0xFFFFFFFF);
    uint64_t mid2 = (t3 & 0xFFFFFFFF);
    uint64_t hi = t4 + (t2 >> 32) + (t3 >> 32);
    
    mid1 += mid2;
    hi += (mid1 >> 32);
    
    uint64_t result = hi + ((mid1 & 0xFFFFFFFF) << 32);
    return result;
}

/* Test function that combines multiple strategies */
NOINLINE static uint64_t test_10_11_operands(int argc, char **argv) {
    uint64_t result = 0;
    
    /* Initialize many variables to use as operands */
    uint64_t vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = (uint64_t)(i + argc) * 123456789;
    }
    
    /* Try different code paths based on argc */
    if (argc > 10) {
        /* Path 1: Complex inline assembly on supported targets */
#ifdef __x86_64__
        result += x86_multi_operand_asm(vars[0], vars[1], vars[2], vars[3],
                                       vars[4], vars[5], vars[6], vars[7],
                                       vars[8], vars[9]);
#endif
    } else if (argc > 5) {
        /* Path 2: Vector operations */
        v4si v1 = {vars[0], vars[1], vars[2], vars[3]};
        v4si v2 = {vars[4], vars[5], vars[6], vars[7]};
        v4si v3 = {vars[8], vars[9], vars[10], vars[11]};
        v4si v4 = {vars[12], vars[13], vars[14], vars[15]};
        v4si v5 = {vars[16], vars[17], vars[18], vars[19]};
        
        v4si vres = vector_complex_op(v1, v2, v3, v4, v5, v1, v2, v3);
        result += vres[0] + vres[1] + vres[2] + vres[3];
    } else {
        /* Path 3: Multi-precision arithmetic */
        for (int i = 0; i < 10; i += 2) {
            result += expand_mult_highpart_test(vars[i], vars[i+1]);
        }
        
        /* Generic multi-operand test */
#ifdef __x86_64__
        /* Fallback to generic if no specific target */
        result += generic_multi_operand(vars[0], vars[1], vars[2], vars[3],
                                       vars[4], vars[5], vars[6], vars[7],
                                       vars[8], vars[9]);
#endif
    }
    
    /* Additional target-specific tests */
#if defined(__x86_64__) && defined(__AVX512F__)
    if (argc > 1) {
        __m512i avx1 = _mm512_set_epi64(vars[0], vars[1], vars[2], vars[3],
                                       vars[4], vars[5], vars[6], vars[7]);
        __m512i avx2 = _mm512_set_epi64(vars[8], vars[9], vars[10], vars[11],
                                       vars[12], vars[13], vars[14], vars[15]);
        __m512i avx3 = _mm512_set_epi64(vars[16], vars[17], vars[18], vars[19],
                                       vars[0], vars[1], vars[2], vars[3]);
        
        __m512i avx_res = avx512_complex_op(avx1, avx2, avx3, avx1, avx2, avx3);
        result += _mm512_extract_epi64(avx_res, 0);
    }
#endif
    
    return result;
}

/* ==================== MAIN FUNCTION ==================== */

int main(int argc, char **argv) {
    uint64_t final_result = 0;
    
    /* Loop to increase coverage with varying inputs */
    for (int iteration = 0; iteration < (argc > 1 ? atoi(argv[1]) % 10 : 3); iteration++) {
        /* Modify argc slightly for different code paths */
        int modified_argc = argc + iteration;
        
        /* Call the test function multiple times */
        final_result += test_10_11_operands(modified_argc, argv);
        
        /* Add some computation to prevent dead code elimination */
        final_result = (final_result * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %lu\n", (unsigned long)final_result);
    
    return (final_result > 0) ? 0 : 1;
}
