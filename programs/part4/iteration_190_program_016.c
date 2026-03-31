/* gcov-optabs-test.c - Test coverage for optabs.cc 10/11 operand expansion */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Simple PRNG to generate non-constant values */
static unsigned int prng_state = 123456789;
static unsigned int prng_next(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function using 10 operands */
int func_10_operands(int a0, int a1, int a2, int a3, int a4,
                     int a5, int a6, int a7, int a8, int a9) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* AVX-512 intrinsic that may expand to many operands */
    #include <immintrin.h>
    __m512i v0 = _mm512_set1_epi32(a0);
    __m512i v1 = _mm512_set1_epi32(a1);
    __m512i v2 = _mm512_set1_epi32(a2);
    __m512i v3 = _mm512_set1_epi32(a3);
    __m512i v4 = _mm512_set1_epi32(a4);
    __m512i v5 = _mm512_set1_epi32(a5);
    __m512i v6 = _mm512_set1_epi32(a6);
    __m512i v7 = _mm512_set1_epi32(a7);
    __m512i v8 = _mm512_set1_epi32(a8);
    __m512i v9 = _mm512_set1_epi32(a9);
    
    /* Complex operation that might require many operands */
    __m512i sum = _mm512_add_epi32(v0, v1);
    sum = _mm512_add_epi32(sum, v2);
    sum = _mm512_add_epi32(sum, v3);
    sum = _mm512_add_epi32(sum, v4);
    sum = _mm512_add_epi32(sum, v5);
    sum = _mm512_add_epi32(sum, v6);
    sum = _mm512_add_epi32(sum, v7);
    sum = _mm512_add_epi32(sum, v8);
    sum = _mm512_add_epi32(sum, v9);
    
    /* Extract result */
    result = _mm512_reduce_add_epi32(sum);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE - use inline asm with many operands */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7),
          "r" (a8), "r" (a9)
        : "cc"
    );
    
#else
    /* Generic fallback: inline asm with 10 input operands */
    asm volatile (
        "# 10-operand operation\n\t"
        "mov %0, %1\n\t"
        "add %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7),
          "r" (a8), "r" (a9)
        : "cc"
    );
#endif
    
    return result;
}

/* Function using 11 operands */
int func_11_operands(int a0, int a1, int a2, int a3, int a4,
                     int a5, int a6, int a7, int a8, int a9,
                     int a10) {
    int result = 0;
    
#if defined(__AVX512F__) && defined(__AVX512VL__)
    /* Another AVX-512 operation with different operand count */
    #include <immintrin.h>
    __m512i v0 = _mm512_set1_epi32(a0);
    __m512i v1 = _mm512_set1_epi32(a1);
    __m512i v2 = _mm512_set1_epi32(a2);
    __m512i v3 = _mm512_set1_epi32(a3);
    __m512i v4 = _mm512_set1_epi32(a4);
    __m512i v5 = _mm512_set1_epi32(a5);
    __m512i v6 = _mm512_set1_epi32(a6);
    __m512i v7 = _mm512_set1_epi32(a7);
    __m512i v8 = _mm512_set1_epi32(a8);
    __m512i v9 = _mm512_set1_epi32(a9);
    __m512i v10 = _mm512_set1_epi32(a10);
    
    /* Different combination to ensure distinct code path */
    __m512i prod = _mm512_mullo_epi32(v0, v1);
    prod = _mm512_add_epi32(prod, v2);
    prod = _mm512_add_epi32(prod, v3);
    prod = _mm512_add_epi32(prod, v4);
    prod = _mm512_add_epi32(prod, v5);
    prod = _mm512_add_epi32(prod, v6);
    prod = _mm512_add_epi32(prod, v7);
    prod = _mm512_add_epi32(prod, v8);
    prod = _mm512_add_epi32(prod, v9);
    prod = _mm512_add_epi32(prod, v10);
    
    result = _mm512_reduce_add_epi32(prod);
    
#elif defined(__ARM_FEATURE_SVE)
    /* ARM SVE with 11 operands */
    asm volatile (
        "mul %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7),
          "r" (a8), "r" (a9), "r" (a10)
        : "cc"
    );
    
#else
    /* Generic fallback: inline asm with 11 input operands */
    asm volatile (
        "# 11-operand operation\n\t"
        "mov %0, %1\n\t"
        "imul %0, %0, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10\n\t"
        "add %0, %0, %11"
        : "=r" (result)
        : "r" (a0), "r" (a1), "r" (a2), "r" (a3),
          "r" (a4), "r" (a5), "r" (a6), "r" (a7),
          "r" (a8), "r" (a9), "r" (a10)
        : "cc"
    );
#endif
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize 11 variables with non-constant values */
    int vars[11];
    
    /* Use argv indices and PRNG to prevent constant propagation */
    for (int i = 0; i < 11; i++) {
        if (i < argc && argv[i] != NULL) {
            vars[i] = atoi(argv[i]) + i;
        } else {
            vars[i] = prng_next() % 100 + 1;
        }
    }
    
    /* Call both functions with overlapping but different operand counts */
    int result1 = func_10_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                                   vars[5], vars[6], vars[7], vars[8], vars[9]);
    
    int result2 = func_11_operands(vars[0], vars[1], vars[2], vars[3], vars[4],
                                   vars[5], vars[6], vars[7], vars[8], vars[9],
                                   vars[10]);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2;
    
    /* Use the result */
    printf("Result: %d\n", final_result);
    
    /* Additional computation to ensure all code paths are used */
    if (final_result > 1000) {
        printf("Large result detected\n");
    }
    
    return final_result != 0 ? 0 : 1;
}
