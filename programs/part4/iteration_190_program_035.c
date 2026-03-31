/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper to generate pseudo-random values from argv */
static unsigned long hash_args(int argc, char **argv) {
    unsigned long h = 0;
    for (int i = 0; i < argc; i++) {
        for (int j = 0; argv[i][j]; j++) {
            h = h * 31 + argv[i][j];
        }
    }
    return h;
}

/* Function to trigger 10-operand expansion */
#ifdef __AVX512F__
#include <immintrin.h>
__m512i func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                         __m512i e, __m512i f, __m512i g, __m512i h,
                         __m512i i, __m512i j) {
    /* Use AVX-512 mask blend with many operands */
    __mmask16 mask = 0xAAAA;
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    result = _mm512_add_epi32(result, c);
    result = _mm512_add_epi32(result, d);
    result = _mm512_add_epi32(result, e);
    result = _mm512_add_epi32(result, f);
    result = _mm512_add_epi32(result, g);
    result = _mm512_add_epi32(result, h);
    result = _mm512_add_epi32(result, i);
    result = _mm512_add_epi32(result, j);
    
    /* Extract and sum elements to create dependency */
    int sum = 0;
    int arr[16];
    _mm512_storeu_si512(arr, result);
    for (int k = 0; k < 16; k++) sum += arr[k];
    return _mm512_set1_epi32(sum);
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
svint32_t func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                          svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                          svint32_t i, svint32_t j) {
    /* SVE predicate with many operands */
    svbool_t pg = svptrue_b32();
    svint32_t result = svadd_m(pg, a, b);
    result = svadd_m(pg, result, c);
    result = svadd_m(pg, result, d);
    result = svadd_m(pg, result, e);
    result = svadd_m(pg, result, f);
    result = svadd_m(pg, result, g);
    result = svadd_m(pg, result, h);
    result = svadd_m(pg, result, i);
    result = svadd_m(pg, result, j);
    return result;
}
#else
/* Generic fallback using inline assembly with 10 operands */
long func_10_operands(long a, long b, long c, long d, long e,
                     long f, long g, long h, long i, long j) {
    long result;
    /* Extended asm with 10 input operands and 1 output */
    asm volatile (
        "add %[a], %[b], %[tmp]\n\t"
        "add %[tmp], %[c], %[tmp]\n\t"
        "add %[tmp], %[d], %[tmp]\n\t"
        "add %[tmp], %[e], %[tmp]\n\t"
        "add %[tmp], %[f], %[tmp]\n\t"
        "add %[tmp], %[g], %[tmp]\n\t"
        "add %[tmp], %[h], %[tmp]\n\t"
        "add %[tmp], %[i], %[tmp]\n\t"
        "add %[tmp], %[j], %[out]"
        : [out] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [tmp] "r" (0L)
        : "cc"
    );
    return result;
}
#endif

/* Function to trigger 11-operand expansion */
#ifdef __AVX512F__
#include <immintrin.h>
__m512i func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                         __m512i e, __m512i f, __m512i g, __m512i h,
                         __m512i i, __m512i j, __m512i k) {
    /* Complex AVX-512 operation with mask and many operands */
    __mmask16 mask1 = 0x5555;
    __mmask16 mask2 = 0xAAAA;
    
    __m512i temp1 = _mm512_mask_blend_epi32(mask1, a, b);
    __m512i temp2 = _mm512_mask_blend_epi32(mask2, c, d);
    __m512i result = _mm512_add_epi32(temp1, temp2);
    
    result = _mm512_add_epi32(result, e);
    result = _mm512_add_epi32(result, f);
    result = _mm512_add_epi32(result, g);
    result = _mm512_add_epi32(result, h);
    result = _mm512_add_epi32(result, i);
    result = _mm512_add_epi32(result, j);
    result = _mm512_add_epi32(result, k);
    
    /* Create result dependency */
    int sum = 0;
    int arr[16];
    _mm512_storeu_si512(arr, result);
    for (int idx = 0; idx < 16; idx++) sum += arr[idx];
    return _mm512_set1_epi32(sum);
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
svint32_t func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                          svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                          svint32_t i, svint32_t j, svint32_t k) {
    /* SVE with multiple predicates and operands */
    svbool_t pg1 = svptrue_pat_b32(SV_VL1);
    svbool_t pg2 = svptrue_pat_b32(SV_VL2);
    
    svint32_t temp1 = svadd_m(pg1, a, b);
    svint32_t temp2 = svadd_m(pg2, c, d);
    svint32_t result = svadd_m(pg1, temp1, temp2);
    
    result = svadd_m(pg1, result, e);
    result = svadd_m(pg1, result, f);
    result = svadd_m(pg1, result, g);
    result = svadd_m(pg1, result, h);
    result = svadd_m(pg1, result, i);
    result = svadd_m(pg1, result, j);
    result = svadd_m(pg1, result, k);
    
    return result;
}
#else
/* Generic fallback using inline assembly with 11 operands */
long func_11_operands(long a, long b, long c, long d, long e,
                     long f, long g, long h, long i, long j, long k) {
    long result;
    /* Extended asm with 11 input operands and 1 output */
    asm volatile (
        "add %[a], %[b], %[tmp1]\n\t"
        "add %[tmp1], %[c], %[tmp2]\n\t"
        "add %[tmp2], %[d], %[tmp1]\n\t"
        "add %[tmp1], %[e], %[tmp2]\n\t"
        "add %[tmp2], %[f], %[tmp1]\n\t"
        "add %[tmp1], %[g], %[tmp2]\n\t"
        "add %[tmp2], %[h], %[tmp1]\n\t"
        "add %[tmp1], %[i], %[tmp2]\n\t"
        "add %[tmp2], %[j], %[tmp1]\n\t"
        "add %[tmp1], %[k], %[out]"
        : [out] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k),
          [tmp1] "r" (0L), [tmp2] "r" (0L)
        : "cc"
    );
    return result;
}
#endif

/* Main function that creates data dependencies */
int main(int argc, char **argv) {
    unsigned long seed = hash_args(argc, argv);
    
    /* Initialize many variables with non-constant values */
    long vals[20];
    for (int i = 0; i < 20; i++) {
        vals[i] = (seed * (i + 1)) ^ (seed >> (i % 32));
    }
    
    /* Call both functions to trigger different expansion paths */
    long result10 = 0, result11 = 0;
    
#ifdef __AVX512F__
    /* For AVX-512, create vector values */
    __m512i vecs[11];
    for (int i = 0; i < 11; i++) {
        int arr[16];
        for (int j = 0; j < 16; j++) {
            arr[j] = (int)(vals[i] ^ (vals[j] << 16));
        }
        vecs[i] = _mm512_loadu_si512((const __m512i*)arr);
    }
    
    __m512i res10 = func_10_operands(vecs[0], vecs[1], vecs[2], vecs[3],
                                    vecs[4], vecs[5], vecs[6], vecs[7],
                                    vecs[8], vecs[9]);
    __m512i res11 = func_11_operands(vecs[0], vecs[1], vecs[2], vecs[3],
                                    vecs[4], vecs[5], vecs[6], vecs[7],
                                    vecs[8], vecs[9], vecs[10]);
    
    /* Extract scalar results */
    int arr10[16], arr11[16];
    _mm512_storeu_si512(arr10, res10);
    _mm512_storeu_si512(arr11, res11);
    for (int i = 0; i < 16; i++) {
        result10 += arr10[i];
        result11 += arr11[i];
    }
#elif defined(__ARM_FEATURE_SVE)
    /* For ARM SVE, use scalar values in vector form */
    result10 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                               vals[4], vals[5], vals[6], vals[7],
                               vals[8], vals[9]);
    result11 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                               vals[4], vals[5], vals[6], vals[7],
                               vals[8], vals[9], vals[10]);
#else
    /* Generic scalar path */
    result10 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                               vals[4], vals[5], vals[6], vals[7],
                               vals[8], vals[9]);
    result11 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                               vals[4], vals[5], vals[6], vals[7],
                               vals[8], vals[9], vals[10]);
#endif
    
    /* Combine results to prevent optimization */
    long final_result = result10 + result11;
    printf("Result: %ld\n", final_result);
    
    return (final_result != 0) ? 0 : 1;
}
