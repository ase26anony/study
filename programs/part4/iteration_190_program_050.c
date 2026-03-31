/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Simple PRNG to generate non-constant values */
static uint64_t simple_rand(uint64_t seed) {
    return seed * 1103515245 + 12345;
}

/* Function using 10 operands */
#ifdef __AVX512F__
#include <immintrin.h>
int64_t func_10_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                         __m512i e, __m512i f, __m512i g, __m512i h,
                         __m512i i, __m512i j) {
    /* AVX-512 intrinsic that may expand to many operands */
    __m512i result = _mm512_add_epi64(a, b);
    result = _mm512_add_epi64(result, c);
    result = _mm512_add_epi64(result, d);
    result = _mm512_add_epi64(result, e);
    result = _mm512_add_epi64(result, f);
    result = _mm512_add_epi64(result, g);
    result = _mm512_add_epi64(result, h);
    result = _mm512_add_epi64(result, i);
    result = _mm512_add_epi64(result, j);
    
    /* Extract and sum elements to create dependency */
    int64_t sum = 0;
    int64_t* ptr = (int64_t*)&result;
    for (int k = 0; k < 8; k++) {
        sum += ptr[k];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
#include <arm_sve.h>
int64_t func_10_operands(svint64_t a, svint64_t b, svint64_t c, svint64_t d,
                         svint64_t e, svint64_t f, svint64_t g, svint64_t h,
                         svint64_t i, svint64_t j) {
    /* SVE intrinsic with predicate - may require many operands */
    svbool_t pg = svptrue_b64();
    svint64_t result = svadd_m(pg, a, b);
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
int64_t func_10_operands(int64_t a, int64_t b, int64_t c, int64_t d,
                         int64_t e, int64_t f, int64_t g, int64_t h,
                         int64_t i, int64_t j) {
    int64_t result1, result2, result3, result4, result5;
    int64_t result6, result7, result8, result9, result10;
    
    /* Extended inline assembly with 10 input operands and 10 output operands */
    asm volatile (
        "mov %0, %10\n\t"
        "add %0, %0, %11\n\t"
        "mov %1, %0\n\t"
        "add %1, %1, %12\n\t"
        "mov %2, %1\n\t"
        "add %2, %2, %13\n\t"
        "mov %3, %2\n\t"
        "add %3, %3, %14\n\t"
        "mov %4, %3\n\t"
        "add %4, %4, %15\n\t"
        "mov %5, %4\n\t"
        "add %5, %5, %16\n\t"
        "mov %6, %5\n\t"
        "add %6, %6, %17\n\t"
        "mov %7, %6\n\t"
        "add %7, %7, %18\n\t"
        "mov %8, %7\n\t"
        "add %8, %8, %19\n\t"
        "mov %9, %8"
        : "=r"(result1), "=r"(result2), "=r"(result3), "=r"(result4),
          "=r"(result5), "=r"(result6), "=r"(result7), "=r"(result8),
          "=r"(result9), "=r"(result10)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h),
          "r"(i), "r"(j)
        : "cc"
    );
    
    /* Create dependency chain */
    return result1 + result2 + result3 + result4 + result5 +
           result6 + result7 + result8 + result9 + result10;
}
#endif

/* Function using 11 operands */
#ifdef __AVX512F__
int64_t func_11_operands(__m512i a, __m512i b, __m512i c, __m512i d,
                         __m512i e, __m512i f, __m512i g, __m512i h,
                         __m512i i, __m512i j, __m512i k) {
    /* Complex AVX-512 operation with mask - may require 11 operands */
    __mmask8 mask = 0xFF;
    __m512i temp1 = _mm512_mask_add_epi64(a, mask, b, c);
    __m512i temp2 = _mm512_mask_add_epi64(d, mask, e, f);
    __m512i temp3 = _mm512_mask_add_epi64(g, mask, h, i);
    __m512i result = _mm512_add_epi64(temp1, temp2);
    result = _mm512_add_epi64(result, temp3);
    result = _mm512_add_epi64(result, j);
    result = _mm512_add_epi64(result, k);
    
    /* Extract and sum elements */
    int64_t sum = 0;
    int64_t* ptr = (int64_t*)&result;
    for (int idx = 0; idx < 8; idx++) {
        sum += ptr[idx];
    }
    return sum;
}
#elif defined(__ARM_FEATURE_SVE)
int64_t func_11_operands(svint64_t a, svint64_t b, svint64_t c, svint64_t d,
                         svint64_t e, svint64_t f, svint64_t g, svint64_t h,
                         svint64_t i, svint64_t j, svint64_t k) {
    /* SVE scatter/gather operation - may require many operands */
    svbool_t pg = svptrue_b64();
    svint64_t indices = svadd_m(pg, a, b);
    svint64_t values = svadd_m(pg, c, d);
    
    /* Complex chain of operations using all 11 operands */
    svint64_t temp1 = svadd_m(pg, e, f);
    svint64_t temp2 = svadd_m(pg, g, h);
    svint64_t temp3 = svadd_m(pg, i, j);
    svint64_t result = svadd_m(pg, temp1, temp2);
    result = svadd_m(pg, result, temp3);
    result = svadd_m(pg, result, k);
    result = svadd_m(pg, result, indices);
    result = svadd_m(pg, result, values);
    
    return svaddv(pg, result);
}
#else
/* Generic fallback using inline assembly with 11 operands */
int64_t func_11_operands(int64_t a, int64_t b, int64_t c, int64_t d,
                         int64_t e, int64_t f, int64_t g, int64_t h,
                         int64_t i, int64_t j, int64_t k) {
    int64_t results[11];
    
    /* Extended inline assembly with 11 input operands */
    asm volatile (
        "mov %0, %11\n\t"
        "add %0, %0, %12\n\t"
        "mov %1, %0\n\t"
        "add %1, %1, %13\n\t"
        "mov %2, %1\n\t"
        "add %2, %2, %14\n\t"
        "mov %3, %2\n\t"
        "add %3, %3, %15\n\t"
        "mov %4, %3\n\t"
        "add %4, %4, %16\n\t"
        "mov %5, %4\n\t"
        "add %5, %5, %17\n\t"
        "mov %6, %5\n\t"
        "add %6, %6, %18\n\t"
        "mov %7, %6\n\t"
        "add %7, %7, %19\n\t"
        "mov %8, %7\n\t"
        "add %8, %8, %20\n\t"
        "mov %9, %8\n\t"
        "add %9, %9, %21\n\t"
        "mov %10, %9"
        : "=r"(results[0]), "=r"(results[1]), "=r"(results[2]),
          "=r"(results[3]), "=r"(results[4]), "=r"(results[5]),
          "=r"(results[6]), "=r"(results[7]), "=r"(results[8]),
          "=r"(results[9]), "=r"(results[10])
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g),
          "r"(h), "r"(i), "r"(j), "r"(k)
        : "cc"
    );
    
    /* Sum all results to create dependency */
    int64_t sum = 0;
    for (int idx = 0; idx < 11; idx++) {
        sum += results[idx];
    }
    return sum;
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize variables with non-constant values */
    uint64_t seed = (argc > 1) ? (uint64_t)argv[1][0] : 123456789;
    
    /* Generate 11 different values */
    int64_t vals[11];
    for (int i = 0; i < 11; i++) {
        seed = simple_rand(seed);
        vals[i] = (int64_t)seed;
    }
    
    /* Call both functions to trigger different expansion paths */
    int64_t result1 = func_10_operands(vals[0], vals[1], vals[2], vals[3],
                                       vals[4], vals[5], vals[6], vals[7],
                                       vals[8], vals[9]);
    
    int64_t result2 = func_11_operands(vals[0], vals[1], vals[2], vals[3],
                                       vals[4], vals[5], vals[6], vals[7],
                                       vals[8], vals[9], vals[10]);
    
    /* Combine results to prevent optimization */
    int64_t final_result = result1 + result2;
    
    /* Print result to ensure side effect */
    printf("Result: %ld\n", (long)final_result);
    
    return (final_result != 0) ? 0 : 1;
}
