/* test_multi_operand_expansion.c
 * Designed to trigger 10 and 11 operand expansion in GCC's optabs.cc
 * Compile with: gcc -O1 -march=native -fdump-rtl-expand test_multi_operand_expansion.c -o test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 1103515245 + 12345;
}

/* Helper to initialize arrays with non-constant values */
static void init_values(uint64_t *arr, int count, uint64_t seed) {
    uint64_t val = seed;
    for (int i = 0; i < count; i++) {
        val = simple_rand(val);
        arr[i] = val;
    }
}

/* Function designed to trigger 10-operand expansion */
#ifdef __AVX512F__
#include <immintrin.h>
uint64_t func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
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
    uint64_t sum = 0;
    int32_t temp[16];
    _mm512_storeu_si512(temp, result);
    for (int k = 0; k < 16; k++) {
        sum += temp[k];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
uint64_t func_10_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                         svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                         svint32_t i, svint32_t j) {
    /* SVE predicate for all active lanes */
    svbool_t pg = svptrue_b32();
    
    /* Chain multiple SVE operations */
    svint32_t result = svadd_m(pg, a, b);
    result = svadd_m(pg, result, c);
    result = svadd_m(pg, result, d);
    result = svadd_m(pg, result, e);
    result = svadd_m(pg, result, f);
    result = svadd_m(pg, result, g);
    result = svadd_m(pg, result, h);
    result = svadd_m(pg, result, i);
    result = svadd_m(pg, result, j);
    
    /* Reduce to scalar */
    return svaddv(pg, result);
}
#else
/* Generic fallback using inline assembly with 10 operands */
uint64_t func_10_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                         uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                         uint64_t i, uint64_t j) {
    uint64_t result1, result2, result3, result4, result5;
    
    /* Extended inline assembly with 10 input operands and 5 outputs */
    asm volatile (
        /* Dummy operations that use all operands */
        "add %[r1], %[a], %[b]\n\t"
        "add %[r2], %[c], %[d]\n\t"
        "add %[r3], %[e], %[f]\n\t"
        "add %[r4], %[g], %[h]\n\t"
        "add %[r5], %[i], %[j]\n\t"
        : [r1] "=r" (result1),
          [r2] "=r" (result2),
          [r3] "=r" (result3),
          [r4] "=r" (result4),
          [r5] "=r" (result5)
        : [a] "r" (a),
          [b] "r" (b),
          [c] "r" (c),
          [d] "r" (d),
          [e] "r" (e),
          [f] "r" (f),
          [g] "r" (g),
          [h] "r" (h),
          [i] "r" (i),
          [j] "r" (j)
        : "cc"
    );
    
    /* Create dependency chain */
    return result1 + result2 + result3 + result4 + result5;
}
#endif

/* Function designed to trigger 11-operand expansion */
#ifdef __AVX512F__
uint64_t func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                         __m512i e, __m512i f, __m512i g, __m512i h,
                         __m512i i, __m512i j, __m512i k) {
    /* Complex AVX-512 operation chain */
    __mmask16 mask1 = 0x5555;
    __mmask16 mask2 = 0xAAAA;
    
    __m512i temp1 = _mm512_mask_blend_epi32(mask1, a, b);
    __m512i temp2 = _mm512_mask_blend_epi32(mask2, c, d);
    __m512i temp3 = _mm512_add_epi32(temp1, temp2);
    __m512i temp4 = _mm512_add_epi32(e, f);
    __m512i temp5 = _mm512_add_epi32(g, h);
    __m512i temp6 = _mm512_add_epi32(i, j);
    
    __m512i result = _mm512_add_epi32(temp3, temp4);
    result = _mm512_add_epi32(result, temp5);
    result = _mm512_add_epi32(result, temp6);
    result = _mm512_add_epi32(result, k);
    
    /* Horizontal sum */
    __m256i sum256 = _mm512_castsi512_si256(
        _mm512_add_epi32(result, _mm512_shuffle_i32x4(result, result, 0x4E)));
    __m128i sum128 = _mm_add_epi32(_mm256_castsi256_si128(sum256),
                                   _mm256_extracti128_si256(sum256, 1));
    sum128 = _mm_add_epi32(sum128, _mm_shuffle_epi32(sum128, 0x4E));
    sum128 = _mm_add_epi32(sum128, _mm_shuffle_epi32(sum128, 0xB1));
    
    return (uint64_t)_mm_cvtsi128_si32(sum128);
}
#elif defined(__ARM_FEATURE_SVE)
uint64_t func_11_operands(svint32_t a, svint32_t b, svint32_t c, svint32_t d,
                         svint32_t e, svint32_t f, svint32_t g, svint32_t h,
                         svint32_t i, svint32_t j, svint32_t k) {
    svbool_t pg = svptrue_b32();
    
    /* Multiple SVE operations with all 11 operands */
    svint32_t temp1 = svadd_m(pg, a, b);
    svint32_t temp2 = svadd_m(pg, c, d);
    svint32_t temp3 = svadd_m(pg, e, f);
    svint32_t temp4 = svadd_m(pg, g, h);
    svint32_t temp5 = svadd_m(pg, i, j);
    
    svint32_t result = svadd_m(pg, temp1, temp2);
    result = svadd_m(pg, result, temp3);
    result = svadd_m(pg, result, temp4);
    result = svadd_m(pg, result, temp5);
    result = svadd_m(pg, result, k);
    
    return svaddv(pg, result);
}
#else
/* Generic fallback with 11 operands */
uint64_t func_11_operands(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                         uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                         uint64_t i, uint64_t j, uint64_t k) {
    uint64_t r1, r2, r3, r4, r5, r6;
    
    /* Inline assembly with 11 input operands */
    asm volatile (
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[r1], %[b]\n\t"
        "mov %[r2], %[c]\n\t"
        "add %[r2], %[r2], %[d]\n\t"
        "mov %[r3], %[e]\n\t"
        "add %[r3], %[r3], %[f]\n\t"
        "mov %[r4], %[g]\n\t"
        "add %[r4], %[r4], %[h]\n\t"
        "mov %[r5], %[i]\n\t"
        "add %[r5], %[r5], %[j]\n\t"
        "mov %[r6], %[k]\n\t"
        "add %[r6], %[r6], %[r1]\n\t"
        : [r1] "=&r" (r1),
          [r2] "=&r" (r2),
          [r3] "=&r" (r3),
          [r4] "=&r" (r4),
          [r5] "=&r" (r5),
          [r6] "=&r" (r6)
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
    
    return r1 + r2 + r3 + r4 + r5 + r6;
}
#endif

int main(int argc, char *argv[]) {
    uint64_t values[20];
    uint64_t seed = (argc > 1) ? (uint64_t)atoi(argv[1]) : 123456789;
    
    /* Initialize with non-constant values */
    init_values(values, 20, seed);
    
    /* For architectures with vector types */
#ifdef __AVX512F__
    __m512i vecs[11];
    for (int i = 0; i < 11; i++) {
        int32_t data[16];
        for (int j = 0; j < 16; j++) {
            data[j] = (int32_t)(values[i + j] & 0xFFFFFFFF);
        }
        vecs[i] = _mm512_loadu_si512((const void *)data);
    }
    
    uint64_t result1 = func_10_operands(vecs[0], vecs[1], vecs[2], vecs[3],
                                       vecs[4], vecs[5], vecs[6], vecs[7],
                                       vecs[8], vecs[9]);
    uint64_t result2 = func_11_operands(vecs[0], vecs[1], vecs[2], vecs[3],
                                       vecs[4], vecs[5], vecs[6], vecs[7],
                                       vecs[8], vecs[9], vecs[10]);
#elif defined(__ARM_FEATURE_SVE)
    /* For ARM SVE - using scalar values as proxy */
    uint64_t result1 = func_10_operands(values[0], values[1], values[2], values[3],
                                       values[4], values[5], values[6], values[7],
                                       values[8], values[9]);
    uint64_t result2 = func_11_operands(values[0], values[1], values[2], values[3],
                                       values[4], values[5], values[6], values[7],
                                       values[8], values[9], values[10]);
#else
    /* Generic scalar path */
    uint64_t result1 = func_10_operands(values[0], values[1], values[2], values[3],
                                       values[4], values[5], values[6], values[7],
                                       values[8], values[9]);
    uint64_t result2 = func_11_operands(values[0], values[1], values[2], values[3],
                                       values[4], values[5], values[6], values[7],
                                       values[8], values[9], values[10]);
#endif
    
    /* Combine results to prevent optimization */
    uint64_t final_result = result1 + result2;
    
    /* Use the result (prevents dead code elimination) */
    printf("Result: %lu\n", (unsigned long)final_result);
    
    /* Additional computation to ensure all code paths are used */
    if (final_result > 1000000) {
        printf("Large result detected\n");
    }
    
    return (final_result == 0) ? 1 : 0;
}
